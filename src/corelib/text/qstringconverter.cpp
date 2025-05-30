/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Copyright (C) 2020 Intel Corporation.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qstringconverter.h>
#include <private/qstringconverter_p.h>
#include "qendian.h"

#include "private/qsimd_p.h"
#include "private/qstringiterator_p.h"
#include "private/qtools_p.h"
#include "qbytearraymatcher.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#ifndef QT_BOOTSTRAPPED
#include <QtCore/qvarlengtharray.h>
#endif // !QT_BOOTSTRAPPED
#else
#ifdef Q_OS_OS2
#include <unicode/ucnv.h>
#endif
#endif

#if __has_include(<bit>) && __cplusplus > 201703L
#include <bit>
#endif

QT_BEGIN_NAMESPACE

enum { Endian = 0, Data = 1 };

static const uchar utf8bom[] = { 0xef, 0xbb, 0xbf };

#if (defined(__SSE2__) && defined(QT_COMPILER_SUPPORTS_SSE2)) \
    || defined(__ARM_NEON__)
static Q_ALWAYS_INLINE uint qBitScanReverse(unsigned v) noexcept
{
#if defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 202002L
     return std::bit_width(v) - 1;
#else
    uint result = qCountLeadingZeroBits(v);
    // Now Invert the result: clz will count *down* from the msb to the lsb, so the msb index is 31
    // and the lsb index is 0. The result for _bit_scan_reverse is expected to be the index when
    // counting up: msb index is 0 (because it starts there), and the lsb index is 31.
    result ^= sizeof(unsigned) * 8 - 1;
    return result;
#endif
}
#endif

#if defined(__SSE2__) && defined(QT_COMPILER_SUPPORTS_SSE2)
static inline bool simdEncodeAscii(uchar *&dst, const ushort *&nextAscii, const ushort *&src, const ushort *end)
{
    // do sixteen characters at a time
    for ( ; end - src >= 16; src += 16, dst += 16) {
#  ifdef __AVX2__
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src));
        __m128i data1 = _mm256_castsi256_si128(data);
        __m128i data2 = _mm256_extracti128_si256(data, 1);
#  else
        __m128i data1 = _mm_loadu_si128((const __m128i*)src);
        __m128i data2 = _mm_loadu_si128(1+(const __m128i*)src);
#  endif

        // check if everything is ASCII
        // the highest ASCII value is U+007F
        // Do the packing directly:
        // The PACKUSWB instruction has packs a signed 16-bit integer to an unsigned 8-bit
        // with saturation. That is, anything from 0x0100 to 0x7fff is saturated to 0xff,
        // while all negatives (0x8000 to 0xffff) get saturated to 0x00. To detect non-ASCII,
        // we simply do a signed greater-than comparison to 0x00. That means we detect NULs as
        // "non-ASCII", but it's an acceptable compromise.
        __m128i packed = _mm_packus_epi16(data1, data2);
        __m128i nonAscii = _mm_cmpgt_epi8(packed, _mm_setzero_si128());

        // store, even if there are non-ASCII characters here
        _mm_storeu_si128((__m128i*)dst, packed);

        // n will contain 1 bit set per character in [data1, data2] that is non-ASCII (or NUL)
        ushort n = ~_mm_movemask_epi8(nonAscii);
        if (n) {
            // find the next probable ASCII character
            // we don't want to load 32 bytes again in this loop if we know there are non-ASCII
            // characters still coming
            nextAscii = src + qBitScanReverse(n) + 1;

            n = qCountTrailingZeroBits(n);
            dst += n;
            src += n;
            return false;
        }
    }

    if (end - src >= 8) {
        // do eight characters at a time
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src));
        __m128i packed = _mm_packus_epi16(data, data);
        __m128i nonAscii = _mm_cmpgt_epi8(packed, _mm_setzero_si128());

        // store even non-ASCII
        _mm_storel_epi64(reinterpret_cast<__m128i *>(dst), packed);

        uchar n = ~_mm_movemask_epi8(nonAscii);
        if (n) {
            nextAscii = src + qBitScanReverse(n) + 1;
            n = qCountTrailingZeroBits(n);
            dst += n;
            src += n;
            return false;
        }
    }

    return src == end;
}

static inline bool simdDecodeAscii(ushort *&dst, const uchar *&nextAscii, const uchar *&src, const uchar *end)
{
    // do sixteen characters at a time
    for ( ; end - src >= 16; src += 16, dst += 16) {
        __m128i data = _mm_loadu_si128((const __m128i*)src);

#ifdef __AVX2__
        const int BitSpacing = 2;
        // load and zero extend to an YMM register
        const __m256i extended = _mm256_cvtepu8_epi16(data);

        uint n = _mm256_movemask_epi8(extended);
        if (!n) {
            // store
            _mm256_storeu_si256((__m256i*)dst, extended);
            continue;
        }
#else
        const int BitSpacing = 1;

        // check if everything is ASCII
        // movemask extracts the high bit of every byte, so n is non-zero if something isn't ASCII
        uint n = _mm_movemask_epi8(data);
        if (!n) {
            // unpack
            _mm_storeu_si128((__m128i*)dst, _mm_unpacklo_epi8(data, _mm_setzero_si128()));
            _mm_storeu_si128(1+(__m128i*)dst, _mm_unpackhi_epi8(data, _mm_setzero_si128()));
            continue;
        }
#endif

        // copy the front part that is still ASCII
        while (!(n & 1)) {
            *dst++ = *src++;
            n >>= BitSpacing;
        }

        // find the next probable ASCII character
        // we don't want to load 16 bytes again in this loop if we know there are non-ASCII
        // characters still coming
        n = qBitScanReverse(n);
        nextAscii = src + (n / BitSpacing) + 1;
        return false;

    }

    if (end - src >= 8) {
        __m128i data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(src));
        uint n = _mm_movemask_epi8(data) & 0xff;
        if (!n) {
            // unpack and store
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dst), _mm_unpacklo_epi8(data, _mm_setzero_si128()));
        } else {
            while (!(n & 1)) {
                *dst++ = *src++;
                n >>= 1;
            }

            n = qBitScanReverse(n);
            nextAscii = src + n + 1;
            return false;
        }
    }

    return src == end;
}

static inline const uchar *simdFindNonAscii(const uchar *src, const uchar *end, const uchar *&nextAscii)
{
#ifdef __AVX2__
    // do 32 characters at a time
    // (this is similar to simdTestMask in qstring.cpp)
    const __m256i mask = _mm256_set1_epi8(char(0x80));
    for ( ; end - src >= 32; src += 32) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src));
        if (_mm256_testz_si256(mask, data))
            continue;

        uint n = _mm256_movemask_epi8(data);
        Q_ASSUME(n);

        // find the next probable ASCII character
        // we don't want to load 32 bytes again in this loop if we know there are non-ASCII
        // characters still coming
        nextAscii = src + qBitScanReverse(n) + 1;

        // return the non-ASCII character
        return src + qCountTrailingZeroBits(n);
    }
#endif

    // do sixteen characters at a time
    for ( ; end - src >= 16; src += 16) {
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));

        // check if everything is ASCII
        // movemask extracts the high bit of every byte, so n is non-zero if something isn't ASCII
        uint n = _mm_movemask_epi8(data);
        if (!n)
            continue;

        // find the next probable ASCII character
        // we don't want to load 16 bytes again in this loop if we know there are non-ASCII
        // characters still coming
        nextAscii = src + qBitScanReverse(n) + 1;

        // return the non-ASCII character
        return src + qCountTrailingZeroBits(n);
    }

    // do four characters at a time
    for ( ; end - src >= 4; src += 4) {
        quint32 data = qFromUnaligned<quint32>(src);
        data &= 0x80808080U;
        if (!data)
            continue;

        // We don't try to guess which of the three bytes is ASCII and which
        // one isn't. The chance that at least two of them are non-ASCII is
        // better than 75%.
        nextAscii = src;
        return src;
    }
    nextAscii = end;
    return src;
}

// Compare only the US-ASCII beginning of [src8, end8) and [src16, end16)
// and advance src8 and src16 to the first character that could not be compared
static void simdCompareAscii(const qchar8_t *&src8, const qchar8_t *end8, const char16_t *&src16, const char16_t *end16)
{
    int bitSpacing = 1;
    qptrdiff len = qMin(end8 - src8, end16 - src16);
    qptrdiff offset = 0;
    uint mask = 0;

    // do sixteen characters at a time
    for ( ; offset + 16 < len; offset += 16) {
        __m128i data8 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src8 + offset));
#ifdef __AVX2__
        // AVX2 version, use 256-bit registers and VPMOVXZBW
        __m256i data16 = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src16 + offset));

        // expand US-ASCII as if it were Latin1 and confirm it's US-ASCII
        __m256i datax8 = _mm256_cvtepu8_epi16(data8);
        mask = _mm256_movemask_epi8(datax8);
        if (mask)
            break;

        // compare Latin1 to UTF-16
        __m256i latin1cmp = _mm256_cmpeq_epi16(datax8, data16);
        mask = ~_mm256_movemask_epi8(latin1cmp);
        if (mask)
            break;
#else
        // non-AVX2 code
        __m128i datalo16 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src16 + offset));
        __m128i datahi16 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src16 + offset) + 1);

        // expand US-ASCII as if it were Latin1, we'll confirm later
        __m128i datalo8 = _mm_unpacklo_epi8(data8, _mm_setzero_si128());
        __m128i datahi8 = _mm_unpackhi_epi8(data8, _mm_setzero_si128());

        // compare Latin1 to UTF-16
        __m128i latin1cmplo = _mm_cmpeq_epi16(datalo8, datalo16);
        __m128i latin1cmphi = _mm_cmpeq_epi16(datahi8, datahi16);
        mask = _mm_movemask_epi8(latin1cmphi) << 16;
        mask |= ushort(_mm_movemask_epi8(latin1cmplo));
        mask = ~mask;
        if (mask)
            break;

        // confirm it was US-ASCII
        mask = _mm_movemask_epi8(data8);
        if (mask) {
            bitSpacing = 0;
            break;
        }
#endif
    }

    // helper for comparing 4 or 8 characters
    auto cmp_lt_16 = [&mask, &offset](int n, __m128i data8, __m128i data16) {
        // n = 4  ->  sizemask = 0xff
        // n = 8  ->  sizemask = 0xffff
        unsigned sizemask = (1U << (2 * n)) - 1;

        // expand as if Latin1
        data8 = _mm_unpacklo_epi8(data8, _mm_setzero_si128());

        // compare and confirm it's US-ASCII
        __m128i latin1cmp = _mm_cmpeq_epi16(data8, data16);
        mask = ~_mm_movemask_epi8(latin1cmp) & sizemask;
        mask |= _mm_movemask_epi8(data8);
        if (mask == 0)
            offset += n;
    };

    // do eight characters at a time
    if (mask == 0 && offset + 8 < len) {
        __m128i data8 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(src8 + offset));
        __m128i data16 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src16 + offset));
        cmp_lt_16(8, data8, data16);
    }

    // do four characters
    if (mask == 0 && offset + 4 < len) {
        __m128i data8 = _mm_cvtsi32_si128(qFromUnaligned<quint32>(src8 + offset));
        __m128i data16 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(src16 + offset));
        cmp_lt_16(4, data8, data16);
    }

    // correct the source pointers to point to the first character we couldn't deal with
    if (mask)
        offset += qCountTrailingZeroBits(mask) >> bitSpacing;
    src8 += offset;
    src16 += offset;
}
#elif defined(__ARM_NEON__)
static inline bool simdEncodeAscii(uchar *&dst, const ushort *&nextAscii, const ushort *&src, const ushort *end)
{
    uint16x8_t maxAscii = vdupq_n_u16(0x7f);
    uint16x8_t mask1 = { 1,      1 << 2, 1 << 4, 1 << 6, 1 << 8, 1 << 10, 1 << 12, 1 << 14 };
    uint16x8_t mask2 = vshlq_n_u16(mask1, 1);

    // do sixteen characters at a time
    for ( ; end - src >= 16; src += 16, dst += 16) {
        // load 2 lanes (or: "load interleaved")
        uint16x8x2_t in = vld2q_u16(src);

        // check if any of the elements > 0x7f, select 1 bit per element (element 0 -> bit 0, element 1 -> bit 1, etc),
        // add those together into a scalar, and merge the scalars.
        uint16_t nonAscii = vaddvq_u16(vandq_u16(vcgtq_u16(in.val[0], maxAscii), mask1))
                          | vaddvq_u16(vandq_u16(vcgtq_u16(in.val[1], maxAscii), mask2));

        // merge the two lanes by shifting the values of the second by 8 and inserting them
        uint16x8_t out = vsliq_n_u16(in.val[0], in.val[1], 8);

        // store, even if there are non-ASCII characters here
        vst1q_u8(dst, vreinterpretq_u8_u16(out));

        if (nonAscii) {
            // find the next probable ASCII character
            // we don't want to load 32 bytes again in this loop if we know there are non-ASCII
            // characters still coming
            nextAscii = src + qBitScanReverse(nonAscii) + 1;

            nonAscii = qCountTrailingZeroBits(nonAscii);
            dst += nonAscii;
            src += nonAscii;
            return false;
        }
    }
    return src == end;
}

static inline bool simdDecodeAscii(ushort *&dst, const uchar *&nextAscii, const uchar *&src, const uchar *end)
{
    // do eight characters at a time
    uint8x8_t msb_mask = vdup_n_u8(0x80);
    uint8x8_t add_mask = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
    for ( ; end - src >= 8; src += 8, dst += 8) {
        uint8x8_t c = vld1_u8(src);
        uint8_t n = vaddv_u8(vand_u8(vcge_u8(c, msb_mask), add_mask));
        if (!n) {
            // store
            vst1q_u16(dst, vmovl_u8(c));
            continue;
        }

        // copy the front part that is still ASCII
        while (!(n & 1)) {
            *dst++ = *src++;
            n >>= 1;
        }

        // find the next probable ASCII character
        // we don't want to load 16 bytes again in this loop if we know there are non-ASCII
        // characters still coming
        n = qBitScanReverse(n);
        nextAscii = src + n + 1;
        return false;

    }
    return src == end;
}

static inline const uchar *simdFindNonAscii(const uchar *src, const uchar *end, const uchar *&nextAscii)
{
    // The SIMD code below is untested, so just force an early return until
    // we've had the time to verify it works.
    nextAscii = end;
    return src;

    // do eight characters at a time
    uint8x8_t msb_mask = vdup_n_u8(0x80);
    uint8x8_t add_mask = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
    for ( ; end - src >= 8; src += 8) {
        uint8x8_t c = vld1_u8(src);
        uint8_t n = vaddv_u8(vand_u8(vcge_u8(c, msb_mask), add_mask));
        if (!n)
            continue;

        // find the next probable ASCII character
        // we don't want to load 16 bytes again in this loop if we know there are non-ASCII
        // characters still coming
        nextAscii = src + qBitScanReverse(n) + 1;

        // return the non-ASCII character
        return src + qCountTrailingZeroBits(n);
    }
    nextAscii = end;
    return src;
}

static void simdCompareAscii(const qchar8_t *&, const qchar8_t *, const char16_t *&, const char16_t *)
{
}
#else
static inline bool simdEncodeAscii(uchar *, const ushort *, const ushort *, const ushort *)
{
    return false;
}

static inline bool simdDecodeAscii(ushort *, const uchar *, const uchar *, const uchar *)
{
    return false;
}

static inline const uchar *simdFindNonAscii(const uchar *src, const uchar *end, const uchar *&nextAscii)
{
    nextAscii = end;
    return src;
}

static void simdCompareAscii(const qchar8_t *&, const qchar8_t *, const char16_t *&, const char16_t *)
{
}
#endif

enum { HeaderDone = 1 };

QByteArray QUtf8::convertFromUnicode(QStringView in)
{
    qsizetype len = in.size();

    // create a QByteArray with the worst case scenario size
    QByteArray result(len * 3, Qt::Uninitialized);
    uchar *dst = reinterpret_cast<uchar *>(const_cast<char *>(result.constData()));
    const ushort *src = reinterpret_cast<const ushort *>(in.data());
    const ushort *const end = src + len;

    while (src != end) {
        const ushort *nextAscii = end;
        if (simdEncodeAscii(dst, nextAscii, src, end))
            break;

        do {
            ushort u = *src++;
            int res = QUtf8Functions::toUtf8<QUtf8BaseTraits>(u, dst, src, end);
            if (res < 0) {
                // encoding error - append '?'
                *dst++ = '?';
            }
        } while (src < nextAscii);
    }

    result.truncate(dst - reinterpret_cast<uchar *>(const_cast<char *>(result.constData())));
    return result;
}

QByteArray QUtf8::convertFromUnicode(QStringView in, QStringConverterBase::State *state)
{
    QByteArray ba(3*in.size() +3, Qt::Uninitialized);
    char *end = convertFromUnicode(ba.data(), in, state);
    ba.truncate(end - ba.data());
    return ba;
}

char *QUtf8::convertFromUnicode(char *out, QStringView in, QStringConverter::State *state)
{
    Q_ASSERT(state);
    const QChar *uc = in.data();
    qsizetype len = in.length();
    if (!len)
        return out;

    auto appendReplacementChar = [state](uchar *cursor) -> uchar * {
        if (state->flags & QStringConverter::Flag::ConvertInvalidToNull) {
            *cursor++ = 0;
        } else {
            // QChar::replacement encoded in utf8
            *cursor++ = 0xef;
            *cursor++ = 0xbf;
            *cursor++ = 0xbd;
        }
        return cursor;
    };

    uchar *cursor = reinterpret_cast<uchar *>(out);
    const ushort *src = reinterpret_cast<const ushort *>(uc);
    const ushort *const end = src + len;

    if (!(state->flags & QStringDecoder::Flag::Stateless)) {
        if (state->remainingChars) {
            int res = QUtf8Functions::toUtf8<QUtf8BaseTraits>(state->state_data[0], cursor, src, end);
            if (res < 0)
                cursor = appendReplacementChar(cursor);
            state->state_data[0] = 0;
            state->remainingChars = 0;
        } else if (!(state->internalState & HeaderDone) && state->flags & QStringConverter::Flag::WriteBom) {
            // append UTF-8 BOM
            *cursor++ = utf8bom[0];
            *cursor++ = utf8bom[1];
            *cursor++ = utf8bom[2];
            state->internalState |= HeaderDone;
        }
    }

    while (src != end) {
        const ushort *nextAscii = end;
        if (simdEncodeAscii(cursor, nextAscii, src, end))
            break;

        do {
            ushort uc = *src++;
            int res = QUtf8Functions::toUtf8<QUtf8BaseTraits>(uc, cursor, src, end);
            if (Q_LIKELY(res >= 0))
                continue;

            if (res == QUtf8BaseTraits::Error) {
                // encoding error
                ++state->invalidChars;
                cursor = appendReplacementChar(cursor);
            } else if (res == QUtf8BaseTraits::EndOfString) {
                if (state->flags & QStringConverter::Flag::Stateless) {
                    ++state->invalidChars;
                    cursor = appendReplacementChar(cursor);
                } else {
                    state->remainingChars = 1;
                    state->state_data[0] = uc;
                }
                return reinterpret_cast<char *>(cursor);
            }
        } while (src < nextAscii);
    }

    return reinterpret_cast<char *>(cursor);
}

QString QUtf8::convertToUnicode(QByteArrayView in)
{
    // UTF-8 to UTF-16 always needs the exact same number of words or less:
    //    UTF-8     UTF-16
    //   1 byte     1 word
    //   2 bytes    1 word
    //   3 bytes    1 word
    //   4 bytes    2 words (one surrogate pair)
    // That is, we'll use the full buffer if the input is US-ASCII (1-byte UTF-8),
    // half the buffer for U+0080-U+07FF text (e.g., Greek, Cyrillic, Arabic) or
    // non-BMP text, and one third of the buffer for U+0800-U+FFFF text (e.g, CJK).
    //
    // The table holds for invalid sequences too: we'll insert one replacement char
    // per invalid byte.
    QString result(in.size(), Qt::Uninitialized);
    QChar *data = const_cast<QChar*>(result.constData()); // we know we're not shared
    const QChar *end = convertToUnicode(data, in);
    result.truncate(end - data);
    return result;
}

/*!
    \since 5.7
    \overload

    Converts the UTF-8 sequence of bytes viewed by \a in to a sequence of
    QChar starting at \a buffer. The buffer is expected to be large enough
    to hold the result. An upper bound for the size of the buffer is
    \c in.size() QChars.

    If, during decoding, an error occurs, a QChar::ReplacementCharacter is
    written.

    Returns a pointer to one past the last QChar written.

    This function never throws.
*/

QChar *QUtf8::convertToUnicode(QChar *buffer, QByteArrayView in) noexcept
{
    ushort *dst = reinterpret_cast<ushort *>(buffer);
    const uchar *const start = reinterpret_cast<const uchar *>(in.data());
    const uchar *src = start;
    const uchar *end = src + in.size();

    // attempt to do a full decoding in SIMD
    const uchar *nextAscii = end;
    if (!simdDecodeAscii(dst, nextAscii, src, end)) {
        // at least one non-ASCII entry
        // check if we failed to decode the UTF-8 BOM; if so, skip it
        if (Q_UNLIKELY(src == start)
                && end - src >= 3
                && Q_UNLIKELY(src[0] == utf8bom[0] && src[1] == utf8bom[1] && src[2] == utf8bom[2])) {
            src += 3;
        }

        while (src < end) {
            nextAscii = end;
            if (simdDecodeAscii(dst, nextAscii, src, end))
                break;

            do {
                uchar b = *src++;
                int res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(b, dst, src, end);
                if (res < 0) {
                    // decoding error
                    *dst++ = QChar::ReplacementCharacter;
                }
            } while (src < nextAscii);
        }
    }

    return reinterpret_cast<QChar *>(dst);
}

QString QUtf8::convertToUnicode(QByteArrayView in, QStringConverter::State *state)
{
    // See above for buffer requirements for stateless decoding. However, that
    // fails if the state is not empty. The following situations can add to the
    // requirements:
    //  state contains      chars starts with           requirement
    //   1 of 2 bytes       valid continuation          0
    //   2 of 3 bytes       same                        0
    //   3 bytes of 4       same                        +1 (need to insert surrogate pair)
    //   1 of 2 bytes       invalid continuation        +1 (need to insert replacement and restart)
    //   2 of 3 bytes       same                        +1 (same)
    //   3 of 4 bytes       same                        +1 (same)
    QString result(in.size() + 1, Qt::Uninitialized);
    QChar *end = convertToUnicode(result.data(), in, state);
    result.truncate(end - result.constData());
    return result;
}

QChar *QUtf8::convertToUnicode(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    qsizetype len = in.size();

    Q_ASSERT(state);
    if (!len)
        return out;


    ushort replacement = QChar::ReplacementCharacter;
    if (state->flags & QStringConverter::Flag::ConvertInvalidToNull)
        replacement = QChar::Null;

    int res;
    uchar ch = 0;

    ushort *dst = reinterpret_cast<ushort *>(out);
    const uchar *src = reinterpret_cast<const uchar *>(in.data());
    const uchar *end = src + len;

    if (!(state->flags & QStringConverter::Flag::Stateless)) {
        bool headerdone = state->internalState & HeaderDone || state->flags & QStringConverter::Flag::ConvertInitialBom;
        if (state->remainingChars || !headerdone) {
            // handle incoming state first
            uchar remainingCharsData[4]; // longest UTF-8 sequence possible
            qsizetype remainingCharsCount = state->remainingChars;
            qsizetype newCharsToCopy = qMin<qsizetype>(sizeof(remainingCharsData) - remainingCharsCount, end - src);

            memset(remainingCharsData, 0, sizeof(remainingCharsData));
            memcpy(remainingCharsData, &state->state_data[0], remainingCharsCount);
            memcpy(remainingCharsData + remainingCharsCount, src, newCharsToCopy);

            const uchar *begin = &remainingCharsData[1];
            res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(remainingCharsData[0], dst, begin,
                    static_cast<const uchar *>(remainingCharsData) + remainingCharsCount + newCharsToCopy);
            if (res == QUtf8BaseTraits::Error) {
                ++state->invalidChars;
                *dst++ = replacement;
                ++src;
            } else if (res == QUtf8BaseTraits::EndOfString) {
                // if we got EndOfString again, then there were too few bytes in src;
                // copy to our state and return
                state->remainingChars = remainingCharsCount + newCharsToCopy;
                memcpy(&state->state_data[0], remainingCharsData, state->remainingChars);
                return out;
            } else if (!headerdone) {
                // eat the UTF-8 BOM
                if (dst[-1] == 0xfeff)
                    --dst;
            }
            state->internalState |= HeaderDone;

            // adjust src now that we have maybe consumed a few chars
            if (res >= 0) {
                Q_ASSERT(res > remainingCharsCount);
                src += res - remainingCharsCount;
            }
        }
    } else if (!(state->flags & QStringConverter::Flag::ConvertInitialBom)) {
        // stateless, remove initial BOM
        if (len > 2 && src[0] == utf8bom[0] && src[1] == utf8bom[1] && src[2] == utf8bom[2])
            // skip BOM
            src += 3;
    }

    // main body, stateless decoding
    res = 0;
    const uchar *nextAscii = src;
    while (res >= 0 && src < end) {
        if (src >= nextAscii && simdDecodeAscii(dst, nextAscii, src, end))
            break;

        ch = *src++;
        res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(ch, dst, src, end);
        if (res == QUtf8BaseTraits::Error) {
            res = 0;
            ++state->invalidChars;
            *dst++ = replacement;
        }
    }

    if (res == QUtf8BaseTraits::EndOfString) {
        // unterminated UTF sequence
        if (state->flags & QStringConverter::Flag::Stateless) {
            *dst++ = QChar::ReplacementCharacter;
            ++state->invalidChars;
            while (src++ < end) {
                *dst++ = QChar::ReplacementCharacter;
                ++state->invalidChars;
            }
            state->remainingChars = 0;
        } else {
            --src; // unread the byte in ch
            state->remainingChars = end - src;
            memcpy(&state->state_data[0], src, end - src);
        }
    } else {
        state->remainingChars = 0;
    }

    return reinterpret_cast<QChar *>(dst);
}

struct QUtf8NoOutputTraits : public QUtf8BaseTraitsNoAscii
{
    struct NoOutput {};
    static void appendUtf16(const NoOutput &, ushort) {}
    static void appendUcs4(const NoOutput &, uint) {}
};

QUtf8::ValidUtf8Result QUtf8::isValidUtf8(QByteArrayView in)
{
    const uchar *src = reinterpret_cast<const uchar *>(in.data());
    const uchar *end = src + in.size();
    const uchar *nextAscii = src;
    bool isValidAscii = true;

    while (src < end) {
        if (src >= nextAscii)
            src = simdFindNonAscii(src, end, nextAscii);
        if (src == end)
            break;

        do {
            uchar b = *src++;
            if ((b & 0x80) == 0)
                continue;

            isValidAscii = false;
            QUtf8NoOutputTraits::NoOutput output;
            int res = QUtf8Functions::fromUtf8<QUtf8NoOutputTraits>(b, output, src, end);
            if (res < 0) {
                // decoding error
                return { false, false };
            }
        } while (src < nextAscii);
    }

    return { true, isValidAscii };
}

int QUtf8::compareUtf8(QByteArrayView utf8, QStringView utf16) noexcept
{
    auto src1 = reinterpret_cast<const qchar8_t *>(utf8.data());
    auto end1 = src1 + utf8.size();
    auto src2 = reinterpret_cast<const char16_t *>(utf16.data());
    auto end2 = src2 + utf16.size();

    do {
        simdCompareAscii(src1, end1, src2, end2);

        if (src1 < end1 && src2 < end2) {
            char32_t uc1 = *src1++;
            char32_t uc2 = *src2++;

            if (uc1 >= 0x80) {
                char32_t *output = &uc1;
                int res = QUtf8Functions::fromUtf8<QUtf8BaseTraitsNoAscii>(uc1, output, src1, end1);
                if (res < 0) {
                    // decoding error
                    uc1 = QChar::ReplacementCharacter;
                }

                // Only decode the UTF-16 surrogate pair if the UTF-8 code point
                // wasn't US-ASCII (a surrogate cannot match US-ASCII).
                if (QChar::isHighSurrogate(uc2) && src2 < end2 && QChar::isLowSurrogate(*src2))
                    uc2 = QChar::surrogateToUcs4(uc2, *src2++);
            }

            if (uc1 != uc2)
                return int(uc1) - int(uc2);
        }
    } while (src1 < end1 && src2 < end2);

    // the shorter string sorts first
    return (end1 > src1) - int(end2 > src2);
}

int QUtf8::compareUtf8(QByteArrayView utf8, QLatin1String s)
{
    uint uc1 = QChar::Null;
    auto src1 = reinterpret_cast<const uchar *>(utf8.data());
    auto end1 = src1 + utf8.size();
    auto src2 = reinterpret_cast<const uchar *>(s.latin1());
    auto end2 = src2 + s.size();

    while (src1 < end1 && src2 < end2) {
        uchar b = *src1++;
        uint *output = &uc1;
        int res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(b, output, src1, end1);
        if (res < 0) {
            // decoding error
            uc1 = QChar::ReplacementCharacter;
        }

        uint uc2 = *src2++;
        if (uc1 != uc2)
            return int(uc1) - int(uc2);
    }

    // the shorter string sorts first
    return (end1 > src1) - (end2 > src2);
}

QByteArray QUtf16::convertFromUnicode(QStringView in, QStringConverter::State *state, DataEndianness endian)
{
    bool writeBom = !(state->internalState & HeaderDone) && state->flags & QStringConverter::Flag::WriteBom;
    qsizetype length = 2 * in.size();
    if (writeBom)
        length += 2;

    QByteArray d(length, Qt::Uninitialized);
    char *end = convertFromUnicode(d.data(), in, state, endian);
    Q_ASSERT(end - d.constData() == d.length());
    Q_UNUSED(end);
    return d;
}

char *QUtf16::convertFromUnicode(char *out, QStringView in, QStringConverter::State *state, DataEndianness endian)
{
    Q_ASSERT(state);
    bool writeBom = !(state->internalState & HeaderDone) && state->flags & QStringConverter::Flag::WriteBom;

    if (endian == DetectEndianness)
        endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;

    if (writeBom) {
        // set them up the BOM
        QChar bom(QChar::ByteOrderMark);
        if (endian == BigEndianness)
            qToBigEndian(bom.unicode(), out);
        else
            qToLittleEndian(bom.unicode(), out);
        out += 2;
    }
    if (endian == BigEndianness)
        qToBigEndian<ushort>(in.data(), in.length(), out);
    else
        qToLittleEndian<ushort>(in.data(), in.length(), out);

    state->remainingChars = 0;
    state->internalState |= HeaderDone;
    return out + 2*in.length();
}

QString QUtf16::convertToUnicode(QByteArrayView in, QStringConverter::State *state, DataEndianness endian)
{
    QString result((in.size() + 1) >> 1, Qt::Uninitialized); // worst case
    QChar *qch = convertToUnicode(result.data(), in, state, endian);
    result.truncate(qch - result.constData());
    return result;
}

QChar *QUtf16::convertToUnicode(QChar *out, QByteArrayView in, QStringConverter::State *state, DataEndianness endian)
{
    qsizetype len = in.size();
    const char *chars = in.data();

    Q_ASSERT(state);

    if (endian == DetectEndianness)
        endian = (DataEndianness)state->state_data[Endian];

    const char *end = chars + len;

    // make sure we can decode at least one char
    if (state->remainingChars + len < 2) {
        if (len) {
            Q_ASSERT(state->remainingChars == 0 && len == 1);
            state->remainingChars = 1;
            state->state_data[Data] = *chars;
        }
        return out;
    }

    bool headerdone = state && state->internalState & HeaderDone;
    if (state->flags & QStringConverter::Flag::ConvertInitialBom)
        headerdone = true;

    if (!headerdone || state->remainingChars) {
        uchar buf;
        if (state->remainingChars)
            buf = state->state_data[Data];
        else
            buf = *chars++;

        // detect BOM, set endianness
        state->internalState |= HeaderDone;
        QChar ch(buf, *chars++);
        if (endian == DetectEndianness) {
            // someone set us up the BOM
            if (ch == QChar::ByteOrderSwapped) {
                endian = BigEndianness;
            } else if (ch == QChar::ByteOrderMark) {
                endian = LittleEndianness;
            } else {
                if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                    endian = BigEndianness;
                } else {
                    endian = LittleEndianness;
                }
            }
        }
        if (endian == BigEndianness)
            ch = QChar::fromUcs2((ch.unicode() >> 8) | ((ch.unicode() & 0xff) << 8));
        if (headerdone || ch != QChar::ByteOrderMark)
            *out++ = ch;
    } else if (endian == DetectEndianness) {
        endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
    }

    qsizetype nPairs = (end - chars) >> 1;
    if (endian == BigEndianness)
        qFromBigEndian<ushort>(chars, nPairs, out);
    else
        qFromLittleEndian<ushort>(chars, nPairs, out);
    out += nPairs;

    state->state_data[Endian] = endian;
    state->remainingChars = 0;
    if ((end - chars) & 1) {
        if (state->flags & QStringConverter::Flag::Stateless) {
            *out++ = state->flags & QStringConverter::Flag::ConvertInvalidToNull ? QChar::Null : QChar::ReplacementCharacter;
        } else {
            state->remainingChars = 1;
            state->state_data[Data] = *(end - 1);
        }
    } else {
        state->state_data[Data] = 0;
    }

    return out;
}

QByteArray QUtf32::convertFromUnicode(QStringView in, QStringConverter::State *state, DataEndianness endian)
{
    bool writeBom = !(state->internalState & HeaderDone) && state->flags & QStringConverter::Flag::WriteBom;
    qsizetype length =  4*in.size();
    if (writeBom)
        length += 4;
    QByteArray ba(length, Qt::Uninitialized);
    char *end = convertFromUnicode(ba.data(), in, state, endian);
    ba.truncate(end - ba.constData());
    return ba;
}

char *QUtf32::convertFromUnicode(char *out, QStringView in, QStringConverter::State *state, DataEndianness endian)
{
    Q_ASSERT(state);

    bool writeBom = !(state->internalState & HeaderDone) && state->flags & QStringConverter::Flag::WriteBom;
    if (endian == DetectEndianness)
        endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;

    if (writeBom) {
        // set them up the BOM
        if (endian == BigEndianness) {
            out[0] = 0;
            out[1] = 0;
            out[2] = (char)0xfe;
            out[3] = (char)0xff;
        } else {
            out[0] = (char)0xff;
            out[1] = (char)0xfe;
            out[2] = 0;
            out[3] = 0;
        }
        out += 4;
        state->internalState |= HeaderDone;
    }

    const QChar *uc = in.data();
    const QChar *end = in.data() + in.length();
    QChar ch;
    uint ucs4;
    if (state->remainingChars == 1) {
        auto character = state->state_data[Data];
        Q_ASSERT(character <= 0xFFFF);
        ch = QChar(character);
        // this is ugly, but shortcuts a whole lot of logic that would otherwise be required
        state->remainingChars = 0;
        goto decode_surrogate;
    }

    while (uc < end) {
        ch = *uc++;
        if (Q_LIKELY(!ch.isSurrogate())) {
            ucs4 = ch.unicode();
        } else if (Q_LIKELY(ch.isHighSurrogate())) {
decode_surrogate:
            if (uc == end) {
                if (state->flags & QStringConverter::Flag::Stateless) {
                    ucs4 = state->flags & QStringConverter::Flag::ConvertInvalidToNull ? 0 : QChar::ReplacementCharacter;
                } else {
                    state->remainingChars = 1;
                    state->state_data[Data] = ch.unicode();
                    return out;
                }
            } else if (uc->isLowSurrogate()) {
                ucs4 = QChar::surrogateToUcs4(ch, *uc++);
            } else {
                ucs4 = state->flags & QStringConverter::Flag::ConvertInvalidToNull ? 0 : QChar::ReplacementCharacter;
            }
        } else {
            ucs4 = state->flags & QStringConverter::Flag::ConvertInvalidToNull ? 0 : QChar::ReplacementCharacter;
        }
        if (endian == BigEndianness)
            qToBigEndian(ucs4, out);
        else
            qToLittleEndian(ucs4, out);
        out += 4;
    }

    return out;
}

QString QUtf32::convertToUnicode(QByteArrayView in, QStringConverter::State *state, DataEndianness endian)
{
    QString result;
    result.resize((in.size() + 7) >> 1); // worst case
    QChar *end = convertToUnicode(result.data(), in, state, endian);
    result.truncate(end - result.constData());
    return result;
}

QChar *QUtf32::convertToUnicode(QChar *out, QByteArrayView in, QStringConverter::State *state, DataEndianness endian)
{
    qsizetype len = in.size();
    const char *chars = in.data();

    Q_ASSERT(state);
    if (endian == DetectEndianness)
        endian = (DataEndianness)state->state_data[Endian];

    const char *end = chars + len;

    uchar tuple[4];
    memcpy(tuple, &state->state_data[Data], 4);

    // make sure we can decode at least one char
    if (state->remainingChars + len < 4) {
        if (len) {
            while (chars < end) {
                tuple[state->remainingChars] = *chars;
                ++state->remainingChars;
                ++chars;
            }
            Q_ASSERT(state->remainingChars < 4);
            memcpy(&state->state_data[Data], tuple, 4);
        }
        return out;
    }

    bool headerdone = state->internalState & HeaderDone;
    if (state->flags & QStringConverter::Flag::ConvertInitialBom)
        headerdone = true;

    qsizetype num = state->remainingChars;
    state->remainingChars = 0;

    if (!headerdone || endian == DetectEndianness || num) {
        while (num < 4)
            tuple[num++] = *chars++;
        if (endian == DetectEndianness) {
            // someone set us up the BOM?
            if (tuple[0] == 0xff && tuple[1] == 0xfe && tuple[2] == 0 && tuple[3] == 0) {
                endian = LittleEndianness;
            } else if (tuple[0] == 0 && tuple[1] == 0 && tuple[2] == 0xfe && tuple[3] == 0xff) {
                endian = BigEndianness;
            } else if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                endian = BigEndianness;
            } else {
                endian = LittleEndianness;
            }
        }
        uint code = (endian == BigEndianness) ? qFromBigEndian<quint32>(tuple) : qFromLittleEndian<quint32>(tuple);
        if (headerdone || code != QChar::ByteOrderMark) {
            if (QChar::requiresSurrogates(code)) {
                *out++ = QChar(QChar::highSurrogate(code));
                *out++ = QChar(QChar::lowSurrogate(code));
            } else {
                *out++ = QChar(code);
            }
        }
        num = 0;
    } else if (endian == DetectEndianness) {
        endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
    }
    state->state_data[Endian] = endian;
    state->internalState |= HeaderDone;

    while (chars < end) {
        tuple[num++] = *chars++;
        if (num == 4) {
            uint code = (endian == BigEndianness) ? qFromBigEndian<quint32>(tuple) : qFromLittleEndian<quint32>(tuple);
            for (char16_t c : QChar::fromUcs4(code))
                *out++ = c;
            num = 0;
        }
    }

    if (num) {
        if (state->flags & QStringDecoder::Flag::Stateless) {
            *out++ = QChar::ReplacementCharacter;
        } else {
            state->state_data[Endian] = endian;
            state->remainingChars = num;
            memcpy(&state->state_data[Data], tuple, 4);
        }
    }

    return out;
}

#if defined(Q_OS_OS2) && !defined(QT_BOOTSTRAPPED)

QString QLocal8Bit::convertToUnicode(QByteArrayView in, QStringConverter::State *state)
{
    const char *src    = in.data();
    qsizetype   srcLen = in.size();
    const char *srcLim = &src[srcLen];

    if (!src || !srcLen)
        return QString();

    Q_ASSERT(srcLen < INT_MAX);
    Q_ASSERT(state);

    // handle a single stateless ASCII character such as a keystroke
    if (srcLen == 1 && ((unsigned char)src[0] < 0x80) && state->remainingChars == 0) {
        char chr[4] = {0};
        chr[0] = src[0];
        return QString(reinterpret_cast<QChar*>(chr));
    }

    // open an ICU converter
    UErrorCode  err = U_ZERO_ERROR;
    UConverter *conv = ucnv_open(0, &err);
    if (U_FAILURE(err)) {
        qWarning("convertToUnicode_sys: ucnv_open() failed - err= %d\n", err);
        return QString();
    }

    // create a target buffer with some slop
    QVarLengthArray<UChar, 4096> ua(srcLen + 16);
    UChar     *tgt    = (UChar*)ua.data();
    qsizetype  tgtLen = ua.size();
    int        len;

    // convert any pending character
    QString sp;
    bool prepend = false;

    // this doesn't consume the 1st char of the source
    // if the conversion fails - should it?
    if (!(state->flags & QStringConverter::Flag::Stateless) &&
        state->remainingChars) {
        state->remainingChars = 0;

        char prev[3] = {0};
        prev[0] = (char)state->state_data[0];
        prev[1] = src[0];
        len = ucnv_toUChars(conv, tgt, tgtLen, prev, 2, &err);

        if (len) {
            sp.append(QChar(tgt[0]));
            if (srcLen == 1) {
                ucnv_close(conv);
                return sp;
            }
            prepend = true;
            src++;
            srcLen--;
            tgt[0] = 0;
        }
    }

    // try the conversion
    ucnv_toUnicode(conv, &tgt, &tgt[tgtLen], &src, srcLim, 0, true, &err);

    // if conversion failed due to buffer overflow, double the size of the buffer
    // and continue where we left off; if that fails, don't even try to recover
    if (U_FAILURE(err)) {
        if (err == U_BUFFER_OVERFLOW_ERROR) {
            tgtLen = tgt - ua.data();
            ua.resize(2 * tgtLen);
            tgt = (UChar*)ua.data();
            tgt += tgtLen;
            err = U_ZERO_ERROR;
            ucnv_toUnicode(conv, &tgt, &tgt[tgtLen],
                           &src, srcLim, NULL, true, &err);
        }
        else
        if (err == U_TRUNCATED_CHAR_FOUND) {
            // the last byte in the buffer is assumed to be
            // the lead byte of a 2-byte sequence - save it
            if (!(state->flags & QStringConverter::Flag::Stateless)) {
                state->state_data[0] = *src;
                state->remainingChars = 1;
            }
        }
        else {
            tgt = ua.data();
            qWarning("convertToUnicode_sys: ucnv_ToUnicode() failed - err= %d\n", err);
        }
    }

    // shut 'er down
    ucnv_close(conv);

    // return an empty string if no chars were output
    len = tgt - ua.data();
    if (len <= 0)
        return QString();

    QString s((QChar*)ua.data(), len);
    if (prepend)
        return sp+s;

    return s;
}

// this is patterned after the Windows implementation which appears to
// ignore the possibility that a surrogate pair could be split across
// two chunks - as such, the conversion is always stateless (we hope)

QByteArray QLocal8Bit::convertFromUnicode(QStringView in, QStringConverter::State *state)
{
    const UChar *src    = (const UChar*)in.data();
    qsizetype    srcLen = in.size();
    const UChar *srcLim = &src[srcLen];

    Q_UNUSED(state);
    Q_ASSERT(srcLen < INT_MAX);

    if (!src)
        return QByteArray();
    if (srcLen == 0)
        return QByteArray("");

    // handle a single ASCII character
    if (srcLen == 1 && src[0] < 0x80)
        return QByteArray(reinterpret_cast<const char*>(src), 1);

    // open an ICU converter
    UErrorCode  err = U_ZERO_ERROR;
    UConverter *conv = ucnv_open(0, &err);
    if (U_FAILURE(err)) {
        qWarning("convertFromUnicode_sys: ucnv_open() failed - err= %d\n", err);
        return QByteArray("");
    }

    // set the default error character to a "mid dot" (0x00B7 in Unicode,
    // 0xFA in CP850) if the current CP supports it, otherwise use a '?'
    char        cpSub[2] = "";
    const UChar uniSub[2] = {0xB7, 0x00};
    if (!ucnv_fromUChars(conv, cpSub, 2, uniSub, 1, &err) || U_FAILURE(err))
        cpSub[0] = '?';
    ucnv_setSubstChars(conv, cpSub, 1, &err);

    // create a target buffer with some slop
    QByteArray   ba(srcLen + 16, 0);
    char        *tgt    = ba.data();
    qsizetype    tgtLen = ba.size();

    // try the conversion; if it fails, double the size of the buffer and
    // continue where we left off; if that fails, don't even try to recover
    ucnv_fromUnicode(conv, &tgt, &tgt[tgtLen],
                     &src, srcLim, NULL, true, &err);

    if (err == U_BUFFER_OVERFLOW_ERROR) {
        tgtLen = tgt - ba.data();
        ba.resize(2 * tgtLen);
        tgt = ba.data();
        tgt += tgtLen;
        err = U_ZERO_ERROR;
        ucnv_fromUnicode(conv, &tgt, &tgt[tgtLen],
                         &src, srcLim, NULL, true, &err);
    }

    // on failure, return an empty buffer; otherwise, resize to actual length
    if (U_FAILURE(err)) {
        ba.resize(0);
        qWarning("convertFromUnicode_sys: ucnv_fromUnicode() failed - err= %d\n", err);
    }
    else {
        ba.resize(tgt - ba.data());
    }

    // close converter & return buffer
    ucnv_close(conv);
    return ba;
}

#else
#if defined(Q_OS_WIN) && !defined(QT_BOOTSTRAPPED)
int QLocal8Bit::checkUtf8()
{
    return GetACP() == CP_UTF8 ? 1 : -1;
}

static QString convertToUnicodeCharByChar(QByteArrayView in, QStringConverter::State *state)
{
    qsizetype length = in.size();
    const char *chars = in.data();

    Q_ASSERT(state);
    if (state->flags & QStringConverter::Flag::Stateless) // temporary
        state = nullptr;

    if (!chars || !length)
        return QString();

    qsizetype copyLocation = 0;
    qsizetype extra = 2;
    if (state && state->remainingChars) {
        copyLocation = state->remainingChars;
        extra += copyLocation;
    }
    qsizetype newLength = length + extra;
    char *mbcs = new char[newLength];
    //ensure that we have a NULL terminated string
    mbcs[newLength-1] = 0;
    mbcs[newLength-2] = 0;
    memcpy(&(mbcs[copyLocation]), chars, length);
    if (copyLocation) {
        //copy the last character from the state
        mbcs[0] = (char)state->state_data[0];
        state->remainingChars = 0;
    }
    const char *mb = mbcs;
    const char *next = 0;
    QString s;
    while ((next = CharNextExA(CP_ACP, mb, 0)) != mb) {
        wchar_t wc[2] ={0};
        int charlength = int(next - mb); // always just a few bytes
        int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, mb, charlength, wc, 2);
        if (len>0) {
            s.append(QChar(wc[0]));
        } else {
            int r = GetLastError();
            //check if the character being dropped is the last character
            if (r == ERROR_NO_UNICODE_TRANSLATION && mb == (mbcs+newLength -3) && state) {
                state->remainingChars = 1;
                state->state_data[0] = (char)*mb;
            }
        }
        mb = next;
    }
    delete [] mbcs;
    return s;
}


QString QLocal8Bit::convertToUnicode_sys(QByteArrayView in, QStringConverter::State *state)
{
    qsizetype length = in.size();

    Q_ASSERT(length < INT_MAX); // ### FIXME
    const char *mb = in.data();
    int mblen = length;

    if (!mb || !mblen)
        return QString();

    QVarLengthArray<wchar_t, 4096> wc(4096);
    int len;
    QString sp;
    bool prepend = false;
    char state_data = 0;
    int remainingChars = 0;

    //save the current state information
    if (state) {
        state_data = (char)state->state_data[0];
        remainingChars = state->remainingChars;
    }

    //convert the pending character (if available)
    if (state && remainingChars) {
        char prev[3] = {0};
        prev[0] = state_data;
        prev[1] = mb[0];
        remainingChars = 0;
        len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                    prev, 2, wc.data(), wc.length());
        if (len) {
            sp.append(QChar(wc[0]));
            if (mblen == 1) {
                state->remainingChars = 0;
                return sp;
            }
            prepend = true;
            mb++;
            mblen--;
            wc[0] = 0;
        }
    }

    while (!(len=MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                mb, mblen, wc.data(), wc.length()))) {
        int r = GetLastError();
        if (r == ERROR_INSUFFICIENT_BUFFER) {
                const int wclen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                    mb, mblen, 0, 0);
                wc.resize(wclen);
        } else if (r == ERROR_NO_UNICODE_TRANSLATION) {
            //find the last non NULL character
            while (mblen > 1  && !(mb[mblen-1]))
                mblen--;
            //check whether,  we hit an invalid character in the middle
            if ((mblen <= 1) || (remainingChars && state_data))
                return convertToUnicodeCharByChar(in, state);
            //Remove the last character and try again...
            state_data = mb[mblen-1];
            remainingChars = 1;
            mblen--;
        } else {
            // Fail.
            qWarning("MultiByteToWideChar: Cannot convert multibyte text");
            break;
        }
    }

    if (len <= 0)
        return QString();

    if (wc[len-1] == 0) // len - 1: we don't want terminator
        --len;

    //save the new state information
    if (state) {
        state->state_data[0] = (char)state_data;
        state->remainingChars = remainingChars;
    }
    QString s((QChar*)wc.data(), len);
    if (prepend) {
        return sp+s;
    }
    return s;
}

QByteArray QLocal8Bit::convertFromUnicode_sys(QStringView in, QStringConverter::State *state)
{
    const QChar *ch = in.data();
    qsizetype uclen = in.size();

    Q_ASSERT(uclen < INT_MAX); // ### FIXME
    Q_ASSERT(state);
    Q_UNUSED(state); // ### Fixme
    if (state->flags & QStringConverter::Flag::Stateless) // temporary
        state = nullptr;

    if (!ch)
        return QByteArray();
    if (uclen == 0)
        return QByteArray("");
    BOOL used_def;
    QByteArray mb(4096, 0);
    int len;
    while (!(len=WideCharToMultiByte(CP_ACP, 0, (const wchar_t*)ch, uclen,
                mb.data(), mb.size()-1, 0, &used_def)))
    {
        int r = GetLastError();
        if (r == ERROR_INSUFFICIENT_BUFFER) {
            mb.resize(1+WideCharToMultiByte(CP_ACP, 0,
                                (const wchar_t*)ch, uclen,
                                0, 0, 0, &used_def));
                // and try again...
        } else {
            // Fail.  Probably can't happen in fact (dwFlags is 0).
#ifndef QT_NO_DEBUG
            // Can't use qWarning(), as it'll recurse to handle %ls
            fprintf(stderr,
                    "WideCharToMultiByte: Cannot convert multibyte text (error %d): %ls\n",
                    r, reinterpret_cast<const wchar_t*>(QString(ch, uclen).utf16()));
#endif
            break;
        }
    }
    mb.resize(len);
    return mb;
}
#endif
#endif

void QStringConverter::State::clear()
{
    if (clearFn)
        clearFn(this);
    else
        state_data[0] = state_data[1] = state_data[2] = state_data[3] = 0;
    remainingChars = 0;
    invalidChars = 0;
    internalState = 0;
}

static QChar *fromUtf16(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf16::convertToUnicode(out, in, state, DetectEndianness);
}

static char *toUtf16(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf16::convertFromUnicode(out, in, state, DetectEndianness);
}

static QChar *fromUtf16BE(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf16::convertToUnicode(out, in, state, BigEndianness);
}

static char *toUtf16BE(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf16::convertFromUnicode(out, in, state, BigEndianness);
}

static QChar *fromUtf16LE(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf16::convertToUnicode(out, in, state, LittleEndianness);
}

static char *toUtf16LE(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf16::convertFromUnicode(out, in, state, LittleEndianness);
}

static QChar *fromUtf32(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf32::convertToUnicode(out, in, state, DetectEndianness);
}

static char *toUtf32(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf32::convertFromUnicode(out, in, state, DetectEndianness);
}

static QChar *fromUtf32BE(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf32::convertToUnicode(out, in, state, BigEndianness);
}

static char *toUtf32BE(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf32::convertFromUnicode(out, in, state, BigEndianness);
}

static QChar *fromUtf32LE(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    return QUtf32::convertToUnicode(out, in, state, LittleEndianness);
}

static char *toUtf32LE(char *out, QStringView in, QStringConverter::State *state)
{
    return QUtf32::convertFromUnicode(out, in, state, LittleEndianness);
}

void qt_from_latin1(char16_t *dst, const char *str, size_t size) noexcept;

static QChar *fromLatin1(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    Q_ASSERT(state);
    Q_UNUSED(state);

    qt_from_latin1(reinterpret_cast<char16_t *>(out), in.data(), size_t(in.size()));
    return out + in.size();
}


static char *toLatin1(char *out, QStringView in, QStringConverter::State *state)
{
    Q_ASSERT(state);
    if (state->flags & QStringConverter::Flag::Stateless) // temporary
        state = nullptr;

    const char replacement = (state && state->flags & QStringConverter::Flag::ConvertInvalidToNull) ? 0 : '?';
    qsizetype invalid = 0;
    for (qsizetype i = 0; i < in.length(); ++i) {
        if (in[i] > QChar(0xff)) {
            *out = replacement;
            ++invalid;
        } else {
            *out = (char)in[i].cell();
        }
        ++out;
    }
    if (state)
        state->invalidChars += invalid;
    return out;
}

static QChar *fromLocal8Bit(QChar *out, QByteArrayView in, QStringConverter::State *state)
{
    QString s = QLocal8Bit::convertToUnicode(in, state);
    memcpy(out, s.constData(), s.length()*sizeof(QChar));
    return out + s.length();
}

static char *toLocal8Bit(char *out, QStringView in, QStringConverter::State *state)
{
    QByteArray s = QLocal8Bit::convertFromUnicode(in, state);
    memcpy(out, s.constData(), s.length());
    return out + s.length();
}


static qsizetype fromUtf8Len(qsizetype l) { return l + 1; }
static qsizetype toUtf8Len(qsizetype l) { return 3*(l + 1); }

static qsizetype fromUtf16Len(qsizetype l) { return l/2 + 2; }
static qsizetype toUtf16Len(qsizetype l) { return 2*(l + 1); }

static qsizetype fromUtf32Len(qsizetype l) { return l/2 + 2; }
static qsizetype toUtf32Len(qsizetype l) { return 4*(l + 1); }

static qsizetype fromLatin1Len(qsizetype l) { return l + 1; }
static qsizetype toLatin1Len(qsizetype l) { return l + 1; }



/*!
  \class QStringConverterBase
  \internal

  Just a common base class for QStringConverter and QTextCodec
*/

/*!
    \class QStringConverter
    \inmodule QtCore
    \brief The QStringConverter class provides a base class for encoding and decoding text.
    \reentrant
    \ingroup i18n

    Qt uses UTF-16 to store, draw and manipulate strings. In many
    situations you may wish to deal with data that uses a different
    encoding. Most text data transferred over files and network connections is encoded
    in UTF-8.

    The QStringConverter class is a base class for the \l {QStringEncoder} and
    \l {QStringDecoder} classes that help with converting between different
    text encodings. QStringDecoder can decode a string from an encoded representation
    into UTF-16, the format Qt uses internally. QStringEncoder does the opposite
    operation, encoding UTF-16 encoded data (usually in the form of a QString) to
    the requested encoding.

    The supported encodings are:

    \list
    \li UTF-8
    \li UTF-16
    \li UTF-16BE
    \li UTF-16LE
    \li UTF-32
    \li UTF-32BE
    \li UTF-32LE
    \li ISO-8859-1 (Latin-1)
    \li The system encoding
    \endlist

    \l {QStringConverter}s can be used as follows to convert some encoded
    string to and from UTF-16.

    Suppose you have some string encoded in UTF-8, and
    want to convert it to a QString. The simple way
    to do it is to use a \l {QStringDecoder} like this:

    \snippet code/src_corelib_text_qstringconverter.cpp 0

    After this, \c string holds the text in decoded form.
    Converting a string from Unicode to the local encoding is just as
    easy using the \l {QStringEncoder} class:

    \snippet code/src_corelib_text_qstringconverter.cpp 1

    To read or write text files in various encodings, use QTextStream and
    its \l{QTextStream::setEncoding()}{setEncoding()} function.

    Some care must be taken when trying to convert the data in chunks,
    for example, when receiving it over a network. In such cases it is
    possible that a multi-byte character will be split over two
    chunks. At best this might result in the loss of a character and
    at worst cause the entire conversion to fail.

    Both QStringEncoder and QStringDecoder make this easy, by tracking
    this in an internal state. So simply calling the encoder or decoder
    again with the next chunk of data will automatically continue encoding
    or decoding the data correctly:

    \snippet code/src_corelib_text_qstringconverter.cpp 2

    The QStringDecoder object maintains state between chunks and therefore
    works correctly even if a multi-byte character is split between
    chunks.

    QStringConverter objects can't be copied because of their internal state, but
    can be moved.

    \sa QTextStream, QStringDecoder, QStringEncoder
*/

/*!
    \enum QStringConverter::Flag

    \value Default Default conversion rules apply.
    \value ConvertInvalidToNull  If this flag is set, each invalid input
                                 character is output as a null character. If it is not set,
                                 invalid input characters are represented as QChar::ReplacementCharacter
                                 if the output encoding can represent that character, otherwise as a question mark.
    \value WriteBom When converting from a QString to an output encoding, write a QChar::ByteOrderMark as the first
                    character if the output encoding supports this. This is the case for UTF-8, UTF-16 and UTF-32
                    encodings.
    \value ConvertInitialBom When converting from an input encoding to a QString the QStringDecoder usually skips an
                              leading QChar::ByteOrderMark. When this flag is set, the byte order mark will not be
                              skipped, but converted to utf-16 and inserted at the start of the created QString.
    \value Stateless Ignore possible converter states between different function calls
           to encode or decode strings. This will also cause the QStringConverter to raise an error if an incomplete
           sequence of data is encountered.
*/

/*!
    \enum QStringConverter::Encoding
    \value Utf8 Create a converter to or from UTF-8
    \value Utf16 Create a converter to or from UTF-16. When decoding, the byte order will get automatically
           detected by a leading byte order mark. If none exists or when encoding, the system byte order will
           be assumed.
    \value Utf16BE Create a converter to or from big-endian UTF-16.
    \value Utf16LE Create a converter to or from little-endian UTF-16.
    \value Utf32 Create a converter to or from UTF-32. When decoding, the byte order will get automatically
           detected by a leading byte order mark. If none exists or when encoding, the system byte order will
           be assumed.
    \value Utf32BE Create a converter to or from big-endian UTF-32.
    \value Utf32LE Create a converter to or from little-endian UTF-32.
    \value Latin1 Create a converter to or from ISO-8859-1 (Latin1).
    \value System Create a converter to or from the underlying encoding of the
           operating systems locale. This is always assumed to be UTF-8 for Unix based
           systems. On Windows, this converts to and from the locale code page.
    \omitvalue LastEncoding
*/

/*!
    \struct QStringConverter::Interface
    \internal
*/

const QStringConverter::Interface QStringConverter::encodingInterfaces[QStringConverter::LastEncoding + 1] =
{
    { "UTF-8", QUtf8::convertToUnicode, fromUtf8Len, QUtf8::convertFromUnicode, toUtf8Len },
    { "UTF-16", fromUtf16, fromUtf16Len, toUtf16, toUtf16Len },
    { "UTF-16LE", fromUtf16LE, fromUtf16Len, toUtf16LE, toUtf16Len },
    { "UTF-16BE", fromUtf16BE, fromUtf16Len, toUtf16BE, toUtf16Len },
    { "UTF-32", fromUtf32, fromUtf32Len, toUtf32, toUtf32Len },
    { "UTF-32LE", fromUtf32LE, fromUtf32Len, toUtf32LE, toUtf32Len },
    { "UTF-32BE", fromUtf32BE, fromUtf32Len, toUtf32BE, toUtf32Len },
    { "ISO-8859-1", fromLatin1, fromLatin1Len, toLatin1, toLatin1Len },
    { "Locale", fromLocal8Bit, fromUtf8Len, toLocal8Bit, toUtf8Len }
};

// match names case insensitive and skipping '-' and '_'
static bool nameMatch(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a == '-' || *a == '_') {
            ++a;
            continue;
        }
        if (*b == '-' || *b == '_') {
            ++b;
            continue;
        }
        if (QtMiscUtils::toAsciiLower(*a) != QtMiscUtils::toAsciiLower(*b))
            return false;
        ++a;
        ++b;
    }
    return !*a && !*b;
}


/*!
    \fn constexpr QStringConverter::QStringConverter()
    \internal
*/

/*!
    \fn constexpr QStringConverter::QStringConverter(Encoding, Flags)
    \internal
*/

/*!
    \internal
*/
QStringConverter::QStringConverter(const char *name, Flags f)
    : iface(nullptr), state(f)
{
    auto e = encodingForName(name);
    if (e)
        iface = encodingInterfaces + int(e.value());
}

/*!
    \fn bool QStringConverter::isValid() const

    Returns true if this is a valid string converter that can be used for encoding or
    decoding text.

    Default constructed string converters or converters constructed with an unsupported
    name are not valid.
*/

/*!
    \fn void QStringConverter::resetState()

    Resets the internal state of the converter, clearing potential errors or partial
    conversions.
*/

/*!
    \fn bool QStringConverter::hasError() const

    Returns true if a conversion could not correctly convert a character. This could for example
    get triggered by an invalid UTF-8 sequence or when a character can't get converted due to
    limitations in the target encoding.
*/

/*!
    \fn const char *QStringConverter::name() const

    Returns the canonical name of the encoding this QStringConverter can encode or decode.
    Returns a nullptr if the converter is not valid.

    \sa isValid()
*/

/*!
    Returns an optional encoding for \a name. The optional is empty if the name could
    not get converted to a valid encoding.
*/
std::optional<QStringConverter::Encoding> QStringConverter::encodingForName(const char *name)
{
    if (!name)
        return std::nullopt;
    for (int i = 0; i < LastEncoding + 1; ++i) {
        if (nameMatch(encodingInterfaces[i].name, name))
            return QStringConverter::Encoding(i);
    }
    if (nameMatch(name, "latin1"))
        return QStringConverter::Latin1;
    return std::nullopt;
}

/*!
   Returns the encoding for the content of \a data if it can be determined.
   \a expectedFirstCharacter can be passed as an additional hint to help determine
   the encoding.

   The returned optional is empty, if the encoding is unclear.
 */
std::optional<QStringConverter::Encoding> QStringConverter::encodingForData(QByteArrayView data, char16_t expectedFirstCharacter)
{
    // someone set us up the BOM?
    qsizetype arraySize = data.size();
    if (arraySize > 3) {
        uint uc = qFromUnaligned<uint>(data.data());
        if (uc == qToBigEndian(uint(QChar::ByteOrderMark)))
            return QStringConverter::Utf32BE;
        if (uc == qToLittleEndian(uint(QChar::ByteOrderMark)))
            return QStringConverter::Utf32LE;
        if (expectedFirstCharacter) {
            // catch also anything starting with the expected character
            if (qToLittleEndian(uc) == expectedFirstCharacter)
                return QStringConverter::Utf32LE;
            else if (qToBigEndian(uc) == expectedFirstCharacter)
                return QStringConverter::Utf32BE;
        }
    }

    if (arraySize > 2) {
        if (memcmp(data.data(), utf8bom, sizeof(utf8bom)) == 0)
            return QStringConverter::Utf8;
    }

    if (arraySize > 1) {
        ushort uc = qFromUnaligned<ushort>(data.data());
        if (uc == qToBigEndian(ushort(QChar::ByteOrderMark)))
            return QStringConverter::Utf16BE;
        if (uc == qToLittleEndian(ushort(QChar::ByteOrderMark)))
            return QStringConverter::Utf16LE;
        if (expectedFirstCharacter) {
            // catch also anything starting with the expected character
            if (qToLittleEndian(uc) == expectedFirstCharacter)
                return QStringConverter::Utf16LE;
            else if (qToBigEndian(uc) == expectedFirstCharacter)
                return QStringConverter::Utf16BE;
        }
    }
    return std::nullopt;
}

/*!
    Tries to determine the encoding of the HTML in \a data by looking at leading byte
    order marks or a charset specifier in the HTML meta tag. If the optional is empty,
    the encoding specified is not supported by QStringConverter. If no encoding is
    detected, the method returns Utf8.
*/
std::optional<QStringConverter::Encoding> QStringConverter::encodingForHtml(QByteArrayView data)
{
    // determine charset
    auto encoding = encodingForData(data);
    if (encoding)
        // trust the initial BOM
        return encoding;

    QByteArray header = data.first(qMin(data.size(), qsizetype(1024))).toByteArray().toLower();
    qsizetype pos = header.indexOf("meta ");
    if (pos != -1) {
        pos = header.indexOf("charset=", pos);
        if (pos != -1) {
            pos += int(qstrlen("charset="));
            if (pos < header.size() && (header.at(pos) == '\"' || header.at(pos) == '\''))
                ++pos;

            qsizetype pos2 = pos;
            // The attribute can be closed with either """, "'", ">" or "/",
            // none of which are valid charset characters.
            while (++pos2 < header.size()) {
                char ch = header.at(pos2);
                if (ch == '\"' || ch == '\'' || ch == '>' || ch == '/') {
                    QByteArray name = header.mid(pos, pos2 - pos);
                    qsizetype colon = name.indexOf(':');
                    if (colon > 0)
                        name = name.left(colon);
                    name = name.simplified();
                    if (name == "unicode") // QTBUG-41998, ICU will return UTF-16.
                        name = QByteArrayLiteral("UTF-8");
                    if (!name.isEmpty())
                        return encodingForName(name);
                }
            }
        }
    }
    return Utf8;
}

/*!
    Returns the canonical name for encoding \a e.
*/
const char *QStringConverter::nameForEncoding(QStringConverter::Encoding e)
{
    return encodingInterfaces[int(e)].name;
}

/*!
    \class QStringEncoder
    \inmodule QtCore
    \brief The QStringEncoder class provides a state-based encoder for text.
    \reentrant
    \ingroup i18n

    A text encoder converts text from Qt's internal representation into an encoded
    text format using a specific encoding.

    Converting a string from Unicode to the local encoding can be achieved
    using the following code:

    \snippet code/src_corelib_text_qstringconverter.cpp 1

    The encoder remembers any state that is required between calls, so converting
    data received in chunks, for example, when receiving it over a network, is just as
    easy, by calling the encoder whenever new data is available:

    \snippet code/src_corelib_text_qstringconverter.cpp 3

    The QStringEncoder object maintains state between chunks and therefore
    works correctly even if a UTF-16 surrogate character is split between
    chunks.

    QStringEncoder objects can't be copied because of their internal state, but
    can be moved.

    \sa QStringConverter, QStringDecoder
*/

/*!
    \fn constexpr QStringEncoder::QStringEncoder(const Interface *i)
    \internal
*/

/*!
    \fn constexpr QStringEncoder::QStringEncoder()

    Default constructs an encoder. The default encoder is not valid,
    and can't be used for converting text.
*/

/*!
    \fn constexpr QStringEncoder::QStringEncoder(Encoding encoding, Flags flags = Flag::Default)

    Creates an encoder object using \a encoding and \a flags.
*/

/*!
    \fn constexpr QStringEncoder::QStringEncoder(const char *name, Flags flags = Flag::Default)

    Creates an encoder object using \a name and \a flags.
    If \a name is not the name of a known encoding an invalid converter will get created.

    \sa isValid()
*/

/*!
    \fn QByteArray QStringEncoder::encode(const QString &in)
    \fn QByteArray QStringEncoder::encode(QStringView in)
    \fn QByteArray QStringEncoder::operator()(const QString &in)
    \fn QByteArray QStringEncoder::operator()(QStringView in)

    Converts \a in and returns the data as a byte array.
*/

/*!
    \fn qsizetype QStringEncoder::requiredSpace(qsizetype inputLength) const

    Returns the maximum amount of characters required to be able to process
    \a inputLength decoded data.

    \sa appendToBuffer()
*/

/*!
    \fn char *QStringEncoder::appendToBuffer(char *out, QStringView in)

    Encodes \a in and writes the encoded result into the buffer
    starting at \a out. Returns a pointer to the end of the data written.

    \note \a out must be large enough to be able to hold all the decoded data. Use
    requiredSpace() to determine the maximum size requirement to be able to encode
    \a in.

    \sa requiredSpace()
*/

/*!
    \class QStringDecoder
    \inmodule QtCore
    \brief The QStringDecoder class provides a state-based decoder for text.
    \reentrant
    \ingroup i18n

    A text decoder converts text an encoded text format that uses a specific encoding
    into Qt's internal representation.

    Converting encoded data into a QString can be achieved
    using the following code:

    \snippet code/src_corelib_text_qstringconverter.cpp 0

    The decoder remembers any state that is required between calls, so converting
    data received in chunks, for example, when receiving it over a network, is just as
    easy, by calling the decoder whenever new data is available:

    \snippet code/src_corelib_text_qstringconverter.cpp 2

    The QStringDecoder object maintains state between chunks and therefore
    works correctly even if chunks are split in the middle of a multi-byte character
    sequence.

    QStringDecoder objects can't be copied because of their internal state, but
    can be moved.

    \sa QStringConverter, QStringEncoder
*/

/*!
    \fn constexpr QStringDecoder::QStringDecoder(const Interface *i)
    \internal
*/

/*!
    \fn constexpr QStringDecoder::QStringDecoder()

    Default constructs an decoder. The default decoder is not valid,
    and can't be used for converting text.
*/

/*!
    \fn constexpr QStringDecoder::QStringDecoder(Encoding encoding, Flags flags = Flag::Default)

    Creates an decoder object using \a encoding and \a flags.
*/

/*!
    \fn constexpr QStringDecoder::QStringDecoder(const char *name, Flags flags = Flag::Default)

    Creates an decoder object using \a name and \a flags.
    If \a name is not the name of a known encoding an invalid converter will get created.

    \sa isValid()
*/

/*!
    \fn QString QStringDecoder::operator()(const QByteArray &ba)
    \fn QString QStringDecoder::decode(const QByteArray &ba)
    \fn QString QStringDecoder::operator()(QByteArrayView ba)
    \fn QString QStringDecoder::decode(QByteArrayView ba)

    Converts \a ba and returns the data as a QString.
*/

/*!
    \fn qsizetype QStringDecoder::requiredSpace(qsizetype inputLength) const

    Returns the maximum amount of UTF-16 code units required to be able to process
    \a inputLength encoded data.

    \sa appendToBuffer
*/

/*!
    \fn QChar *QStringDecoder::appendToBuffer(QChar *out, QByteArrayView in)

    Decodes the sequence of bytes viewed by \a in and writes the decoded result into
    the buffer starting at \a out. Returns a pointer to the end of data written.

    \a out needs to be large enough to be able to hold all the decoded data. Use
    \l{requiredSpace} to determine the maximum size requirements to decode an encoded
    data buffer of \c in.size() bytes.

    \sa requiredSpace
*/

QT_END_NAMESPACE
