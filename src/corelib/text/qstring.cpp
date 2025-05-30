/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Copyright (C) 2022 Intel Corporation.
** Copyright (C) 2019 Mail.ru Group.
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

#include "qstringlist.h"
#if QT_CONFIG(regularexpression)
#include "qregularexpression.h"
#endif
#include "qunicodetables_p.h"
#include <private/qstringconverter_p.h>
#include "qlocale_tools_p.h"
#include "private/qsimd_p.h"
#include <qnumeric.h>
#include <qdatastream.h>
#include <qlist.h>
#include "qlocale.h"
#include "qlocale_p.h"
#include "qstringbuilder.h"
#include "qstringmatcher.h"
#include "qvarlengtharray.h"
#include "qdebug.h"
#include "qendian.h"
#include "qcollator.h"

#ifdef Q_OS_MAC
#include <private/qcore_mac_p.h>
#endif

#include <private/qfunctions_p.h>

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#include "qchar.cpp"
#include "qstringmatcher.cpp"
#include "qstringiterator_p.h"
#include "qstringalgorithms_p.h"
#include "qthreadstorage.h"

#ifdef Q_OS_WIN
#  include <qt_windows.h>
#endif

#ifdef truncate
#  undef truncate
#endif

#ifndef LLONG_MAX
#define LLONG_MAX qint64_C(9223372036854775807)
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - qint64_C(1))
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX quint64_C(18446744073709551615)
#endif

#define IS_RAW_DATA(d) ((d.d)->flags & QArrayData::RawDataType)

QT_BEGIN_NAMESPACE

const char16_t QString::_empty = 0;

/*
 * Note on the use of SIMD in qstring.cpp:
 *
 * Several operations with strings are improved with the use of SIMD code,
 * since they are repetitive. For MIPS, we have hand-written assembly code
 * outside of qstring.cpp targeting MIPS DSP and MIPS DSPr2. For ARM and for
 * x86, we can only use intrinsics and therefore everything is contained in
 * qstring.cpp. We need to use intrinsics only for those platforms due to the
 * different compilers and toolchains used, which have different syntax for
 * assembly sources.
 *
 * ** SSE notes: **
 *
 * Whenever multiple alternatives are equivalent or near so, we prefer the one
 * using instructions from SSE2, since SSE2 is guaranteed to be enabled for all
 * 64-bit builds and we enable it for 32-bit builds by default. Use of higher
 * SSE versions should be done when there is a clear performance benefit and
 * requires fallback code to SSE2, if it exists.
 *
 * Performance measurement in the past shows that most strings are short in
 * size and, therefore, do not benefit from alignment prologues. That is,
 * trying to find a 16-byte-aligned boundary to operate on is often more
 * expensive than executing the unaligned operation directly. In addition, note
 * that the QString private data is designed so that the data is stored on
 * 16-byte boundaries if the system malloc() returns 16-byte aligned pointers
 * on its own (64-bit glibc on Linux does; 32-bit glibc on Linux returns them
 * 50% of the time), so skipping the alignment prologue is actually optimizing
 * for the common case.
 */

#if defined(__mips_dsp)
// From qstring_mips_dsp_asm.S
extern "C" void qt_fromlatin1_mips_asm_unroll4 (char16_t*, const char*, uint);
extern "C" void qt_fromlatin1_mips_asm_unroll8 (char16_t*, const char*, uint);
extern "C" void qt_toLatin1_mips_dsp_asm(uchar *dst, const char16_t *src, int length);
#endif

// internal
qsizetype qFindStringBoyerMoore(QStringView haystack, qsizetype from, QStringView needle, Qt::CaseSensitivity cs);
static inline qsizetype qFindChar(QStringView str, QChar ch, qsizetype from, Qt::CaseSensitivity cs) noexcept;
template <typename Haystack>
static inline qsizetype qLastIndexOf(Haystack haystack, QChar needle, qsizetype from, Qt::CaseSensitivity cs) noexcept;
template <>
inline qsizetype qLastIndexOf(QString haystack, QChar needle,
                              qsizetype from, Qt::CaseSensitivity cs) noexcept = delete; // unwanted, would detach

static inline bool qt_starts_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs);
static inline bool qt_starts_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs);
static inline bool qt_ends_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs);

#if defined(__SSE2__) && defined(Q_CC_GNU) && !defined(Q_CC_INTEL)
#  if defined(__SANITIZE_ADDRESS__) && Q_CC_GNU < 800 && !defined(Q_CC_CLANG)
#     warning "The __attribute__ on below will likely cause a build failure with your GCC version. Your choices are:"
#     warning "1) disable ASan;"
#     warning "2) disable the optimized code in qustrlen (change __SSE2__ to anything else);"
#     warning "3) upgrade your compiler (preferred)."
#  endif

// We may overrun the buffer, but that's a false positive:
// this won't crash nor produce incorrect results
__attribute__((__no_sanitize_address__))
#endif
qsizetype QtPrivate::qustrlen(const char16_t *str) noexcept
{
    qsizetype result = 0;

#if defined(__SSE2__) && !(defined(__SANITIZE_ADDRESS__) || __has_feature(address_sanitizer))
    // find the 16-byte alignment immediately prior or equal to str
    quintptr misalignment = quintptr(str) & 0xf;
    Q_ASSERT((misalignment & 1) == 0);
    const char16_t *ptr = str - (misalignment / 2);

    // load 16 bytes and see if we have a null
    // (aligned loads can never segfault)
    const __m128i zeroes = _mm_setzero_si128();
    __m128i data = _mm_load_si128(reinterpret_cast<const __m128i *>(ptr));
    __m128i comparison = _mm_cmpeq_epi16(data, zeroes);
    quint32 mask = _mm_movemask_epi8(comparison);

    // ignore the result prior to the beginning of str
    mask >>= misalignment;

    // Have we found something in the first block? Need to handle it now
    // because of the left shift above.
    if (mask)
        return qCountTrailingZeroBits(quint32(mask)) / 2;

    do {
        ptr += 8;
        data = _mm_load_si128(reinterpret_cast<const __m128i *>(ptr));

        comparison = _mm_cmpeq_epi16(data, zeroes);
        mask = _mm_movemask_epi8(comparison);
    } while (mask == 0);

    // found a null
    uint idx = qCountTrailingZeroBits(quint32(mask));
    return ptr - str + idx / 2;
#endif

    if (sizeof(wchar_t) == sizeof(char16_t))
        return wcslen(reinterpret_cast<const wchar_t *>(str));

    while (*str++)
        ++result;
    return result;
}

#if !defined(__OPTIMIZE_SIZE__)
namespace {
template <uint MaxCount> struct UnrollTailLoop
{
    template <typename RetType, typename Functor1, typename Functor2, typename Number>
    static inline RetType exec(Number count, RetType returnIfExited, Functor1 loopCheck, Functor2 returnIfFailed, Number i = 0)
    {
        /* equivalent to:
         *   while (count--) {
         *       if (loopCheck(i))
         *           return returnIfFailed(i);
         *   }
         *   return returnIfExited;
         */

        if (!count)
            return returnIfExited;

        bool check = loopCheck(i);
        if (check)
            return returnIfFailed(i);

        return UnrollTailLoop<MaxCount - 1>::exec(count - 1, returnIfExited, loopCheck, returnIfFailed, i + 1);
    }

    template <typename Functor, typename Number>
    static inline void exec(Number count, Functor code)
    {
        /* equivalent to:
         *   for (Number i = 0; i < count; ++i)
         *       code(i);
         */
        exec(count, 0, [=](Number i) -> bool { code(i); return false; }, [](Number) { return 0; });
    }
};
template <> template <typename RetType, typename Functor1, typename Functor2, typename Number>
inline RetType UnrollTailLoop<0>::exec(Number, RetType returnIfExited, Functor1, Functor2, Number)
{
    return returnIfExited;
}
}
#endif

/*!
 * \internal
 *
 * Searches for character \a c in the string \a str and returns a pointer to
 * it. Unlike strchr() and wcschr() (but like glibc's strchrnul()), if the
 * character is not found, this function returns a pointer to the end of the
 * string -- that is, \c{str.end()}.
 */
const char16_t *QtPrivate::qustrchr(QStringView str, char16_t c) noexcept
{
    const char16_t *n = str.utf16();
    const char16_t *e = n + str.size();

#ifdef __SSE2__
    bool loops = true;
    // Using the PMOVMSKB instruction, we get two bits for each character
    // we compare.
#  if defined(__AVX2__) && !defined(__OPTIMIZE_SIZE__)
    // we're going to read n[0..15] (32 bytes)
    __m256i mch256 = _mm256_set1_epi32(c | (c << 16));
    for (const char16_t *next = n + 16; next <= e; n = next, next += 16) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(n));
        __m256i result = _mm256_cmpeq_epi16(data, mch256);
        uint mask = uint(_mm256_movemask_epi8(result));
        if (mask) {
            uint idx = qCountTrailingZeroBits(mask);
            return n + idx / 2;
        }
    }
    loops = false;
    __m128i mch = _mm256_castsi256_si128(mch256);
#  else
    __m128i mch = _mm_set1_epi32(c | (c << 16));
#  endif

    auto hasMatch = [mch, &n](__m128i data, ushort validityMask) {
        __m128i result = _mm_cmpeq_epi16(data, mch);
        uint mask = uint(_mm_movemask_epi8(result));
        if ((mask & validityMask) == 0)
            return false;
        uint idx = qCountTrailingZeroBits(mask);
        n += idx / 2;
        return true;
    };

    // we're going to read n[0..7] (16 bytes)
    for (const char16_t *next = n + 8; next <= e; n = next, next += 8) {
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(n));
        if (hasMatch(data, 0xffff))
            return n;

        if (!loops) {
            n += 8;
            break;
        }
    }

#  if !defined(__OPTIMIZE_SIZE__)
    // we're going to read n[0..3] (8 bytes)
    if (e - n > 3) {
        __m128i data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(n));
        if (hasMatch(data, 0xff))
            return n;

        n += 4;
    }

    return UnrollTailLoop<3>::exec(e - n, e,
                                   [=](int i) { return n[i] == c; },
                                   [=](int i) { return n + i; });
#  endif
#elif defined(__ARM_NEON__)
    const uint16x8_t vmask = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
    const uint16x8_t ch_vec = vdupq_n_u16(c);
    for (const char16_t *next = n + 8; next <= e; n = next, next += 8) {
        uint16x8_t data = vld1q_u16(reinterpret_cast<const uint16_t *>(n));
        uint mask = vaddvq_u16(vandq_u16(vceqq_u16(data, ch_vec), vmask));
        if (ushort(mask)) {
            // found a match
            return n + qCountTrailingZeroBits(mask);
        }
    }
#endif // aarch64

    --n;
    while (++n != e)
        if (*n == c)
            return n;

    return n;
}

#ifdef __SSE2__
// Scans from \a ptr to \a end until \a maskval is non-zero. Returns true if
// the no non-zero was found. Returns false and updates \a ptr to point to the
// first 16-bit word that has any bit set (note: if the input is 8-bit, \a ptr
// may be updated to one byte short).
static bool simdTestMask(const char *&ptr, const char *end, quint32 maskval)
{
    auto updatePtr = [&](uint result) {
        // found a character matching the mask
        uint idx = qCountTrailingZeroBits(~result);
        ptr += idx;
        return false;
    };

#  if defined(__SSE4_1__)
    __m128i mask;
    auto updatePtrSimd = [&](__m128i data) {
        __m128i masked = _mm_and_si128(mask, data);
        __m128i comparison = _mm_cmpeq_epi16(masked, _mm_setzero_si128());
        uint result = _mm_movemask_epi8(comparison);
        return updatePtr(result);
    };

#    if defined(__AVX2__)
    // AVX2 implementation: test 32 bytes at a time
    const __m256i mask256 = _mm256_broadcastd_epi32(_mm_cvtsi32_si128(maskval));
    while (ptr + 32 <= end) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(ptr));
        if (!_mm256_testz_si256(mask256, data)) {
            // found a character matching the mask
            __m256i masked256 = _mm256_and_si256(mask256, data);
            __m256i comparison256 = _mm256_cmpeq_epi16(masked256, _mm256_setzero_si256());
            return updatePtr(_mm256_movemask_epi8(comparison256));
        }
        ptr += 32;
    }

    mask = _mm256_castsi256_si128(mask256);
#    else
    // SSE 4.1 implementation: test 32 bytes at a time (two 16-byte
    // comparisons, unrolled)
    mask = _mm_set1_epi32(maskval);
    while (ptr + 32 <= end) {
        __m128i data1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ptr));
        __m128i data2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ptr + 16));
        if (!_mm_testz_si128(mask, data1))
            return updatePtrSimd(data1);

        ptr += 16;
        if (!_mm_testz_si128(mask, data2))
            return updatePtrSimd(data2);
        ptr += 16;
    }
#    endif

    // AVX2 and SSE4.1: final 16-byte comparison
    if (ptr + 16 <= end) {
        __m128i data1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ptr));
        if (!_mm_testz_si128(mask, data1))
            return updatePtrSimd(data1);
        ptr += 16;
    }

    // and final 8-byte comparison
    if (ptr + 8 <= end) {
        __m128i data1 = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(ptr));
        if (!_mm_testz_si128(mask, data1))
            return updatePtrSimd(data1);
        ptr += 8;
    }

#  else
    // SSE2 implementation: test 16 bytes at a time.
    const __m128i mask = _mm_set1_epi32(maskval);
    while (ptr + 16 <= end) {
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ptr));
        __m128i masked = _mm_and_si128(mask, data);
        __m128i comparison = _mm_cmpeq_epi16(masked, _mm_setzero_si128());
        quint16 result = _mm_movemask_epi8(comparison);
        if (result != 0xffff)
            return updatePtr(result);
        ptr += 16;
    }

    // and one 8-byte comparison
    if (ptr + 8 <= end) {
        __m128i data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(ptr));
        __m128i masked = _mm_and_si128(mask, data);
        __m128i comparison = _mm_cmpeq_epi16(masked, _mm_setzero_si128());
        quint8 result = _mm_movemask_epi8(comparison);
        if (result != 0xff)
            return updatePtr(result);
        ptr += 8;
    }
#  endif

    return true;
}

static Q_ALWAYS_INLINE __m128i mm_load8_zero_extend(const void *ptr)
{
    const __m128i *dataptr = static_cast<const __m128i *>(ptr);
#if defined(__SSE4_1__)
    // use a MOVQ followed by PMOVZXBW
    // if AVX2 is present, these should combine into a single VPMOVZXBW instruction
    __m128i data = _mm_loadl_epi64(dataptr);
    return _mm_cvtepu8_epi16(data);
#  else
    // use MOVQ followed by PUNPCKLBW
    __m128i data = _mm_loadl_epi64(dataptr);
    return _mm_unpacklo_epi8(data, _mm_setzero_si128());
#  endif
}
#endif

// Note: ptr on output may be off by one and point to a preceding US-ASCII
// character. Usually harmless.
bool qt_is_ascii(const char *&ptr, const char *end) noexcept
{
#if defined(__SSE2__)
    // Testing for the high bit can be done efficiently with just PMOVMSKB
    bool loops = true;
#  if defined(__AVX2__)
    while (ptr + 32 <= end) {
        __m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(ptr));
        quint32 mask = _mm256_movemask_epi8(data);
        if (mask) {
            uint idx = qCountTrailingZeroBits(mask);
            ptr += idx;
            return false;
        }
        ptr += 32;
    }
    loops = false;
#  endif

    while (ptr + 16 <= end) {
        __m128i data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ptr));
        quint32 mask = _mm_movemask_epi8(data);
        if (mask) {
            uint idx = qCountTrailingZeroBits(mask);
            ptr += idx;
            return false;
        }
        ptr += 16;

        if (!loops)
            break;
    }
    if (ptr + 8 <= end) {
        __m128i data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(ptr));
        quint8 mask = _mm_movemask_epi8(data);
        if (mask) {
            uint idx = qCountTrailingZeroBits(mask);
            ptr += idx;
            return false;
        }
        ptr += 8;
    }
#endif

    while (ptr + 4 <= end) {
        quint32 data = qFromUnaligned<quint32>(ptr);
        if (data &= 0x80808080U) {
            uint idx = QSysInfo::ByteOrder == QSysInfo::BigEndian
                    ? qCountLeadingZeroBits(data)
                    : qCountTrailingZeroBits(data);
            ptr += idx / 8;
            return false;
        }
        ptr += 4;
    }

    while (ptr != end) {
        if (quint8(*ptr) & 0x80)
            return false;
        ++ptr;
    }
    return true;
}

bool QtPrivate::isAscii(QLatin1String s) noexcept
{
    const char *ptr = s.begin();
    const char *end = s.end();

    return qt_is_ascii(ptr, end);
}

static bool isAscii_helper(const char16_t *&ptr, const char16_t *end)
{
#ifdef __SSE2__
    const char *ptr8 = reinterpret_cast<const char *>(ptr);
    const char *end8 = reinterpret_cast<const char *>(end);
    bool ok = simdTestMask(ptr8, end8, 0xff80ff80);
    ptr = reinterpret_cast<const char16_t *>(ptr8);
    if (!ok)
        return false;
#endif

    while (ptr != end) {
        if (*ptr & 0xff80)
            return false;
        ++ptr;
    }
    return true;
}

bool QtPrivate::isAscii(QStringView s) noexcept
{
    const char16_t *ptr = s.utf16();
    const char16_t *end = ptr + s.size();

    return isAscii_helper(ptr, end);
}

bool QtPrivate::isLatin1(QStringView s) noexcept
{
    const char16_t *ptr = s.utf16();
    const char16_t *end = ptr + s.size();

#ifdef __SSE2__
    const char *ptr8 = reinterpret_cast<const char *>(ptr);
    const char *end8 = reinterpret_cast<const char *>(end);
    if (!simdTestMask(ptr8, end8, 0xff00ff00))
        return false;
    ptr = reinterpret_cast<const char16_t *>(ptr8);
#endif

    while (ptr != end) {
        if (*ptr++ > 0xff)
            return false;
    }
    return true;
}

bool QtPrivate::isValidUtf16(QStringView s) noexcept
{
    constexpr char32_t InvalidCodePoint = UINT_MAX;

    QStringIterator i(s);
    while (i.hasNext()) {
        const char32_t c = i.next(InvalidCodePoint);
        if (c == InvalidCodePoint)
            return false;
    }

    return true;
}

// conversion between Latin 1 and UTF-16
Q_CORE_EXPORT void qt_from_latin1(char16_t *dst, const char *str, size_t size) noexcept
{
    /* SIMD:
     * Unpacking with SSE has been shown to improve performance on recent CPUs
     * The same method gives no improvement with NEON. On Aarch64, clang will do the vectorization
     * itself in exactly the same way as one would do it with intrinsics.
     */
#if defined(__SSE2__)
    const char *e = str + size;
    qptrdiff offset = 0;

    // we're going to read str[offset..offset+15] (16 bytes)
    for ( ; str + offset + 15 < e; offset += 16) {
        const __m128i chunk = _mm_loadu_si128((const __m128i*)(str + offset)); // load
#ifdef __AVX2__
        // zero extend to an YMM register
        const __m256i extended = _mm256_cvtepu8_epi16(chunk);

        // store
        _mm256_storeu_si256((__m256i*)(dst + offset), extended);
#else
        const __m128i nullMask = _mm_set1_epi32(0);

        // unpack the first 8 bytes, padding with zeros
        const __m128i firstHalf = _mm_unpacklo_epi8(chunk, nullMask);
        _mm_storeu_si128((__m128i*)(dst + offset), firstHalf); // store

        // unpack the last 8 bytes, padding with zeros
        const __m128i secondHalf = _mm_unpackhi_epi8 (chunk, nullMask);
        _mm_storeu_si128((__m128i*)(dst + offset + 8), secondHalf); // store
#endif
    }

    // we're going to read str[offset..offset+7] (8 bytes)
    if (str + offset + 7 < e) {
        const __m128i unpacked = mm_load8_zero_extend(str + offset);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(dst + offset), unpacked);
        offset += 8;
    }

    size = size % 8;
    dst += offset;
    str += offset;
#  if !defined(__OPTIMIZE_SIZE__)
    return UnrollTailLoop<7>::exec(int(size), [=](int i) { dst[i] = (uchar)str[i]; });
#  endif
#endif
#if defined(__mips_dsp)
    static_assert(sizeof(qsizetype) == sizeof(int),
                  "oops, the assembler implementation needs to be called in a loop");
    if (size > 20)
        qt_fromlatin1_mips_asm_unroll8(dst, str, size);
    else
        qt_fromlatin1_mips_asm_unroll4(dst, str, size);
#else
    while (size--)
        *dst++ = (uchar)*str++;
#endif
}

template <bool Checked>
static void qt_to_latin1_internal(uchar *dst, const char16_t *src, qsizetype length)
{
#if defined(__SSE2__)
    uchar *e = dst + length;
    qptrdiff offset = 0;

#  ifdef __AVX2__
    const __m256i questionMark256 = _mm256_broadcastw_epi16(_mm_cvtsi32_si128('?'));
    const __m256i outOfRange256 = _mm256_broadcastw_epi16(_mm_cvtsi32_si128(0x100));
    const __m128i questionMark = _mm256_castsi256_si128(questionMark256);
    const __m128i outOfRange = _mm256_castsi256_si128(outOfRange256);
#  else
    const __m128i questionMark = _mm_set1_epi16('?');
    const __m128i outOfRange = _mm_set1_epi16(0x100);
#  endif

    auto mergeQuestionMarks = [=](__m128i chunk) {
        // SSE has no compare instruction for unsigned comparison.
# ifdef __SSE4_1__
        // We use an unsigned uc = qMin(uc, 0x100) and then compare for equality.
        chunk = _mm_min_epu16(chunk, outOfRange);
        const __m128i offLimitMask = _mm_cmpeq_epi16(chunk, outOfRange);
        chunk = _mm_blendv_epi8(chunk, questionMark, offLimitMask);
# else
        // The variables must be shiffted + 0x8000 to be compared
        const __m128i signedBitOffset = _mm_set1_epi16(short(0x8000));
        const __m128i thresholdMask = _mm_set1_epi16(short(0xff + 0x8000));

        const __m128i signedChunk = _mm_add_epi16(chunk, signedBitOffset);
        const __m128i offLimitMask = _mm_cmpgt_epi16(signedChunk, thresholdMask);

        // offLimitQuestionMark contains '?' for each 16 bits that was off-limit
        // the 16 bits that were correct contains zeros
        const __m128i offLimitQuestionMark = _mm_and_si128(offLimitMask, questionMark);

        // correctBytes contains the bytes that were in limit
        // the 16 bits that were off limits contains zeros
        const __m128i correctBytes = _mm_andnot_si128(offLimitMask, chunk);

        // merge offLimitQuestionMark and correctBytes to have the result
        chunk = _mm_or_si128(correctBytes, offLimitQuestionMark);

        Q_UNUSED(outOfRange);
# endif
        return chunk;
    };

    // we're going to write to dst[offset..offset+15] (16 bytes)
    for ( ; dst + offset + 15 < e; offset += 16) {
#  if defined(__AVX2__)
        __m256i chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(src + offset));
        if (Checked) {
            // See mergeQuestionMarks lambda above for details
            chunk = _mm256_min_epu16(chunk, outOfRange256);
            const __m256i offLimitMask = _mm256_cmpeq_epi16(chunk, outOfRange256);
            chunk = _mm256_blendv_epi8(chunk, questionMark256, offLimitMask);
        }

        const __m128i chunk2 = _mm256_extracti128_si256(chunk, 1);
        const __m128i chunk1 = _mm256_castsi256_si128(chunk);
#  else
        __m128i chunk1 = _mm_loadu_si128((const __m128i*)(src + offset)); // load
        if (Checked)
            chunk1 = mergeQuestionMarks(chunk1);

        __m128i chunk2 = _mm_loadu_si128((const __m128i*)(src + offset + 8)); // load
        if (Checked)
            chunk2 = mergeQuestionMarks(chunk2);
#  endif

        // pack the two vector to 16 x 8bits elements
        const __m128i result = _mm_packus_epi16(chunk1, chunk2);
        _mm_storeu_si128((__m128i*)(dst + offset), result); // store
    }

#  if !defined(__OPTIMIZE_SIZE__)
    // we're going to write to dst[offset..offset+7] (8 bytes)
    if (dst + offset + 7 < e) {
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + offset));
        if (Checked)
            chunk = mergeQuestionMarks(chunk);

        // pack, where the upper half is ignored
        const __m128i result = _mm_packus_epi16(chunk, chunk);
        _mm_storel_epi64(reinterpret_cast<__m128i *>(dst + offset), result);
        offset += 8;
    }

    // we're going to write to dst[offset..offset+3] (4 bytes)
    if (dst + offset + 3 < e) {
        __m128i chunk = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(src + offset));
        if (Checked)
            chunk = mergeQuestionMarks(chunk);

        // pack, we'll the upper three quarters
        const __m128i result = _mm_packus_epi16(chunk, chunk);
        qToUnaligned(_mm_cvtsi128_si32(result), dst + offset);
        offset += 4;
    }

    length = length % 4;
#  else
    length = length % 16;
#  endif // optimize size

    // advance dst, src for tail processing
    dst += offset;
    src += offset;

#  if !defined(__OPTIMIZE_SIZE__)
    return UnrollTailLoop<3>::exec(length, [=](int i) {
        if (Checked)
            dst[i] = (src[i]>0xff) ? '?' : (uchar) src[i];
        else
            dst[i] = src[i];
    });
#  endif
#elif defined(__ARM_NEON__)
    // Refer to the documentation of the SSE2 implementation.
    // This uses exactly the same method as for SSE except:
    // 1) neon has unsigned comparison
    // 2) packing is done to 64 bits (8 x 8bits component).
    if (length >= 16) {
        const qsizetype chunkCount = length >> 3; // divided by 8
        const uint16x8_t questionMark = vdupq_n_u16('?'); // set
        const uint16x8_t thresholdMask = vdupq_n_u16(0xff); // set
        for (qsizetype i = 0; i < chunkCount; ++i) {
            uint16x8_t chunk = vld1q_u16((uint16_t *)src); // load
            src += 8;

            if (Checked) {
                const uint16x8_t offLimitMask = vcgtq_u16(chunk, thresholdMask); // chunk > thresholdMask
                const uint16x8_t offLimitQuestionMark = vandq_u16(offLimitMask, questionMark); // offLimitMask & questionMark
                const uint16x8_t correctBytes = vbicq_u16(chunk, offLimitMask); // !offLimitMask & chunk
                chunk = vorrq_u16(correctBytes, offLimitQuestionMark); // correctBytes | offLimitQuestionMark
            }
            const uint8x8_t result = vmovn_u16(chunk); // narrowing move->packing
            vst1_u8(dst, result); // store
            dst += 8;
        }
        length = length % 8;
    }
#endif
#if defined(__mips_dsp)
    static_assert(sizeof(qsizetype) == sizeof(int),
                  "oops, the assembler implementation needs to be called in a loop");
    qt_toLatin1_mips_dsp_asm(dst, src, length);
#else
    while (length--) {
        if (Checked)
            *dst++ = (*src>0xff) ? '?' : (uchar) *src;
        else
            *dst++ = *src;
        ++src;
    }
#endif
}

static void qt_to_latin1(uchar *dst, const char16_t *src, qsizetype length)
{
    qt_to_latin1_internal<true>(dst, src, length);
}

void qt_to_latin1_unchecked(uchar *dst, const char16_t *src, qsizetype length)
{
    qt_to_latin1_internal<false>(dst, src, length);
}

// Unicode case-insensitive comparison
static int ucstricmp(const QChar *a, const QChar *ae, const QChar *b, const QChar *be)
{
    if (a == b)
        return (ae - be);

    const QChar *e = ae;
    if (be - b < ae - a)
        e = a + (be - b);

    char32_t alast = 0;
    char32_t blast = 0;
    while (a < e) {
//         qDebug() << Qt::hex << alast << blast;
//         qDebug() << Qt::hex << "*a=" << *a << "alast=" << alast << "folded=" << foldCase (*a, alast);
//         qDebug() << Qt::hex << "*b=" << *b << "blast=" << blast << "folded=" << foldCase (*b, blast);
        int diff = foldCase(a->unicode(), alast) - foldCase(b->unicode(), blast);
        if ((diff))
            return diff;
        ++a;
        ++b;
    }
    if (a == ae) {
        if (b == be)
            return 0;
        return -1;
    }
    return 1;
}

// Case-insensitive comparison between a Unicode string and a QLatin1String
static int ucstricmp(const QChar *a, const QChar *ae, const char *b, const char *be)
{
    auto e = ae;
    if (be - b < ae - a)
        e = a + (be - b);

    while (a < e) {
        int diff = foldCase(a->unicode()) - foldCase(char16_t{uchar(*b)});
        if ((diff))
            return diff;
        ++a;
        ++b;
    }
    if (a == ae) {
        if (b == be)
            return 0;
        return -1;
    }
    return 1;
}

// Case-insensitive comparison between a Unicode string and a UTF-8 string
static int ucstricmp8(const char *utf8, const char *utf8end, const QChar *utf16, const QChar *utf16end)
{
    auto src1 = reinterpret_cast<const uchar *>(utf8);
    auto end1 = reinterpret_cast<const uchar *>(utf8end);
    QStringIterator src2(utf16, utf16end);

    while (src1 < end1 && src2.hasNext()) {
        uint uc1 = 0;
        uint *output = &uc1;
        uchar b = *src1++;
        int res = QUtf8Functions::fromUtf8<QUtf8BaseTraits>(b, output, src1, end1);
        if (res < 0) {
            // decoding error
            uc1 = QChar::ReplacementCharacter;
        } else {
            uc1 = QChar::toCaseFolded(uc1);
        }

        uint uc2 = QChar::toCaseFolded(src2.next());
        int diff = uc1 - uc2;   // can't underflow
        if (diff)
            return diff;
    }

    // the shorter string sorts first
    return (end1 > src1) - int(src2.hasNext());
}

#if defined(__mips_dsp)
// From qstring_mips_dsp_asm.S
extern "C" int qt_ucstrncmp_mips_dsp_asm(const char16_t *a,
                                         const char16_t *b,
                                         unsigned len);
#endif

// Unicode case-sensitive compare two same-sized strings
static int ucstrncmp(const QChar *a, const QChar *b, size_t l)
{
#ifdef __OPTIMIZE_SIZE__
    const QChar *end = a + l;
    while (a < end) {
        if (int diff = (int)a->unicode() - (int)b->unicode())
            return diff;
        ++a;
        ++b;
    }
    return 0;
#else
#if defined(__mips_dsp)
    static_assert(sizeof(uint) == sizeof(size_t));
    if (l >= 8) {
        return qt_ucstrncmp_mips_dsp_asm(reinterpret_cast<const char16_t*>(a),
                                         reinterpret_cast<const char16_t*>(b),
                                         l);
    }
#endif // __mips_dsp
#ifdef __SSE2__
    const QChar *end = a + l;
    qptrdiff offset = 0;

    // Using the PMOVMSKB instruction, we get two bits for each character
    // we compare.
    int retval;
    auto isDifferent = [a, b, &offset, &retval](__m128i a_data, __m128i b_data) {
        __m128i result = _mm_cmpeq_epi16(a_data, b_data);
        uint mask = ~uint(_mm_movemask_epi8(result));
        if (ushort(mask) == 0)
            return false;
        uint idx = qCountTrailingZeroBits(mask);
        retval = a[offset + idx / 2].unicode() - b[offset + idx / 2].unicode();
        return true;
    };

    // we're going to read a[0..15] and b[0..15] (32 bytes)
    for ( ; end - a >= offset + 16; offset += 16) {
#ifdef __AVX2__
        __m256i a_data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + offset));
        __m256i b_data = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(b + offset));
        __m256i result = _mm256_cmpeq_epi16(a_data, b_data);
        uint mask = _mm256_movemask_epi8(result);
#else
        __m128i a_data1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a + offset));
        __m128i a_data2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a + offset + 8));
        __m128i b_data1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b + offset));
        __m128i b_data2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b + offset + 8));
        __m128i result1 = _mm_cmpeq_epi16(a_data1, b_data1);
        __m128i result2 = _mm_cmpeq_epi16(a_data2, b_data2);
        uint mask = _mm_movemask_epi8(result1) | (_mm_movemask_epi8(result2) << 16);
#endif
        mask = ~mask;
        if (mask) {
            // found a different character
            uint idx = qCountTrailingZeroBits(mask);
            return a[offset + idx / 2].unicode() - b[offset + idx / 2].unicode();
        }
    }

    // we're going to read a[0..7] and b[0..7] (16 bytes)
    if (end - a >= offset + 8) {
        __m128i a_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(a + offset));
        __m128i b_data = _mm_loadu_si128(reinterpret_cast<const __m128i *>(b + offset));
        if (isDifferent(a_data, b_data))
            return retval;

        offset += 8;
    }

    // we're going to read a[0..3] and b[0..3] (8 bytes)
    if (end - a >= offset + 4) {
        __m128i a_data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(a + offset));
        __m128i b_data = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(b + offset));
        if (isDifferent(a_data, b_data))
            return retval;

        offset += 4;
    }

    // reset l
    l &= 3;

    const auto lambda = [=](size_t i) -> int {
        return a[offset + i].unicode() - b[offset + i].unicode();
    };
    return UnrollTailLoop<3>::exec(l, 0, lambda, lambda);
#endif
#ifdef __ARM_NEON__
    if (l >= 8) {
        const QChar *end = a + l;
        const uint16x8_t mask = { 1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
        while (end - a > 7) {
            uint16x8_t da = vld1q_u16(reinterpret_cast<const uint16_t *>(a));
            uint16x8_t db = vld1q_u16(reinterpret_cast<const uint16_t *>(b));

            uint8_t r = ~(uint8_t)vaddvq_u16(vandq_u16(vceqq_u16(da, db), mask));
            if (r) {
                // found a different QChar
                uint idx = qCountTrailingZeroBits(r);
                return (int)a[idx].unicode() - (int)b[idx].unicode();
            }
            a += 8;
            b += 8;
        }
        l &= 7;
    }
    const auto lambda = [=](size_t i) -> int {
        return a[i].unicode() - b[i].unicode();
    };
    return UnrollTailLoop<7>::exec(l, 0, lambda, lambda);
#endif // __ARM_NEON__
    if (!l)
        return 0;

    // check alignment
    if ((reinterpret_cast<quintptr>(a) & 2) == (reinterpret_cast<quintptr>(b) & 2)) {
        // both addresses have the same alignment
        if (reinterpret_cast<quintptr>(a) & 2) {
            // both addresses are not aligned to 4-bytes boundaries
            // compare the first character
            if (*a != *b)
                return a->unicode() - b->unicode();
            --l;
            ++a;
            ++b;

            // now both addresses are 4-bytes aligned
        }

        // both addresses are 4-bytes aligned
        // do a fast 32-bit comparison
        const quint32 *da = reinterpret_cast<const quint32 *>(a);
        const quint32 *db = reinterpret_cast<const quint32 *>(b);
        const quint32 *e = da + (l >> 1);
        for ( ; da != e; ++da, ++db) {
            if (*da != *db) {
                a = reinterpret_cast<const QChar *>(da);
                b = reinterpret_cast<const QChar *>(db);
                if (*a != *b)
                    return a->unicode() - b->unicode();
                return a[1].unicode() - b[1].unicode();
            }
        }

        // do we have a tail?
        a = reinterpret_cast<const QChar *>(da);
        b = reinterpret_cast<const QChar *>(db);
        return (l & 1) ? a->unicode() - b->unicode() : 0;
    } else {
        // one of the addresses isn't 4-byte aligned but the other is
        const QChar *e = a + l;
        for ( ; a != e; ++a, ++b) {
            if (*a != *b)
                return a->unicode() - b->unicode();
        }
    }
    return 0;
#endif
}

static int ucstrncmp(const QChar *a, const uchar *c, size_t l)
{
    const char16_t *uc = reinterpret_cast<const char16_t *>(a);
    const char16_t *e = uc + l;

#ifdef __SSE2__
    __m128i nullmask = _mm_setzero_si128();
    qptrdiff offset = 0;

#  if !defined(__OPTIMIZE_SIZE__)
    // Using the PMOVMSKB instruction, we get two bits for each character
    // we compare.
    int retval;
    auto isDifferent = [uc, c, &offset, &retval](__m128i a_data, __m128i b_data) {
        __m128i result = _mm_cmpeq_epi16(a_data, b_data);
        uint mask = ~uint(_mm_movemask_epi8(result));
        if (ushort(mask) == 0)
            return false;
        uint idx = qCountTrailingZeroBits(mask);
        retval = uc[offset + idx / 2] - c[offset + idx / 2];
        return true;
    };
#  endif

    // we're going to read uc[offset..offset+15] (32 bytes)
    // and c[offset..offset+15] (16 bytes)
    for ( ; uc + offset + 15 < e; offset += 16) {
        // similar to fromLatin1_helper:
        // load 16 bytes of Latin 1 data
        __m128i chunk = _mm_loadu_si128((const __m128i*)(c + offset));

#  ifdef __AVX2__
        // expand Latin 1 data via zero extension
        __m256i ldata = _mm256_cvtepu8_epi16(chunk);

        // load UTF-16 data and compare
        __m256i ucdata = _mm256_loadu_si256((const __m256i*)(uc + offset));
        __m256i result = _mm256_cmpeq_epi16(ldata, ucdata);

        uint mask = ~_mm256_movemask_epi8(result);
#  else
        // expand via unpacking
        __m128i firstHalf = _mm_unpacklo_epi8(chunk, nullmask);
        __m128i secondHalf = _mm_unpackhi_epi8(chunk, nullmask);

        // load UTF-16 data and compare
        __m128i ucdata1 = _mm_loadu_si128((const __m128i*)(uc + offset));
        __m128i ucdata2 = _mm_loadu_si128((const __m128i*)(uc + offset + 8));
        __m128i result1 = _mm_cmpeq_epi16(firstHalf, ucdata1);
        __m128i result2 = _mm_cmpeq_epi16(secondHalf, ucdata2);

        uint mask = ~(_mm_movemask_epi8(result1) | _mm_movemask_epi8(result2) << 16);
#  endif
        if (mask) {
            // found a different character
            uint idx = qCountTrailingZeroBits(mask);
            return uc[offset + idx / 2] - c[offset + idx / 2];
        }
    }

#  if !defined(__OPTIMIZE_SIZE__)
    // we'll read uc[offset..offset+7] (16 bytes) and c[offset..offset+7] (8 bytes)
    if (uc + offset + 7 < e) {
        // same, but we're using an 8-byte load
        __m128i secondHalf = mm_load8_zero_extend(c + offset);

        __m128i ucdata = _mm_loadu_si128((const __m128i*)(uc + offset));
        if (isDifferent(ucdata, secondHalf))
            return retval;

        // still matched
        offset += 8;
    }

    enum { MaxTailLength = 3 };
    // we'll read uc[offset..offset+3] (8 bytes) and c[offset..offset+3] (4 bytes)
    if (uc + offset + 3 < e) {
        __m128i chunk = _mm_cvtsi32_si128(qFromUnaligned<int>(c + offset));
        __m128i secondHalf = _mm_unpacklo_epi8(chunk, nullmask);

        __m128i ucdata = _mm_loadl_epi64(reinterpret_cast<const __m128i *>(uc + offset));
        if (isDifferent(ucdata, secondHalf))
            return retval;

        // still matched
        offset += 4;
    }
#  endif // optimize size

    // reset uc and c
    uc += offset;
    c += offset;

#  if !defined(__OPTIMIZE_SIZE__)
    const auto lambda = [=](size_t i) { return uc[i] - char16_t(c[i]); };
    return UnrollTailLoop<MaxTailLength>::exec(e - uc, 0, lambda, lambda);
#  endif
#endif

    while (uc < e) {
        int diff = *uc - *c;
        if (diff)
            return diff;
        uc++, c++;
    }

    return 0;
}

constexpr int lencmp(qsizetype lhs, qsizetype rhs) noexcept
{
    return lhs == rhs ? 0 :
           lhs >  rhs ? 1 :
           /* else */  -1 ;
}

// Unicode case-sensitive comparison
static int ucstrcmp(const QChar *a, size_t alen, const QChar *b, size_t blen)
{
    if (a == b && alen == blen)
        return 0;
    const size_t l = qMin(alen, blen);
    int cmp = ucstrncmp(a, b, l);
    return cmp ? cmp : lencmp(alen, blen);
}

static int ucstrcmp(const QChar *a, size_t alen, const char *b, size_t blen)
{
    const size_t l = qMin(alen, blen);
    const int cmp = ucstrncmp(a, reinterpret_cast<const uchar*>(b), l);
    return cmp ? cmp : lencmp(alen, blen);
}

static int latin1nicmp(const char *lhsChar, qsizetype lSize, const char *rhsChar, qsizetype rSize)
{
    constexpr uchar latin1Lower[256] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
        0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
        0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
        0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x5b,0x5c,0x5d,0x5e,0x5f,
        0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
        0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
        0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
        0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
        0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
        0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
        // 0xd7 (multiplication sign) and 0xdf (sz ligature) complicate life
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xd7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xdf,
        0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
        0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
    };
    // We're called with QLatin1String's .data() and .size():
    Q_ASSERT(lSize >= 0 && rSize >= 0);
    if (!lSize)
        return rSize ? -1 : 0;
    if (!rSize)
        return 1;
    const qsizetype size = std::min(lSize, rSize);

    const uchar *lhs = reinterpret_cast<const uchar *>(lhsChar);
    const uchar *rhs = reinterpret_cast<const uchar *>(rhsChar);
    Q_ASSERT(lhs && rhs); // since both lSize and rSize are positive
    for (qsizetype i = 0; i < size; i++) {
        Q_ASSERT(lhs[i] && rhs[i]);
        if (int res = latin1Lower[lhs[i]] - latin1Lower[rhs[i]])
            return res;
    }
    return lencmp(lSize, rSize);
}
bool QtPrivate::equalStrings(QStringView lhs, QStringView rhs) noexcept
{
    return ucstrcmp(lhs.begin(), lhs.size(), rhs.begin(), rhs.size()) == 0;
}

bool QtPrivate::equalStrings(QStringView lhs, QLatin1String rhs) noexcept
{
    return ucstrcmp(lhs.begin(), lhs.size(), rhs.begin(), rhs.size()) == 0;
}

bool QtPrivate::equalStrings(QLatin1String lhs, QStringView rhs) noexcept
{
    return QtPrivate::equalStrings(rhs, lhs);
}

bool QtPrivate::equalStrings(QLatin1String lhs, QLatin1String rhs) noexcept
{
    return lhs.size() == rhs.size() && (!lhs.size() || qstrncmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

bool QtPrivate::equalStrings(QBasicUtf8StringView<false> lhs, QStringView rhs) noexcept
{
    return QUtf8::compareUtf8(lhs, rhs) == 0;
}

bool QtPrivate::equalStrings(QStringView lhs, QBasicUtf8StringView<false> rhs) noexcept
{
    return QtPrivate::equalStrings(rhs, lhs);
}

bool QtPrivate::equalStrings(QLatin1String lhs, QBasicUtf8StringView<false> rhs) noexcept
{
    QString r = rhs.toString();
    return QtPrivate::equalStrings(lhs, r); // ### optimize!
}

bool QtPrivate::equalStrings(QBasicUtf8StringView<false> lhs, QLatin1String rhs) noexcept
{
    return QtPrivate::equalStrings(rhs, lhs);
}

bool QtPrivate::equalStrings(QBasicUtf8StringView<false> lhs, QBasicUtf8StringView<false> rhs) noexcept
{
    return lhs.size() == rhs.size() && (!lhs.size() || qstrncmp(lhs.data(), rhs.data(), lhs.size()) == 0);
}

bool QAnyStringView::equal(QAnyStringView lhs, QAnyStringView rhs) noexcept
{
    if (lhs.size() != rhs.size() && lhs.isUtf8() == rhs.isUtf8())
        return false;
    return lhs.visit([rhs](auto lhs) {
        return rhs.visit([lhs](auto rhs) {
            return QtPrivate::equalStrings(lhs, rhs);
        });
    });
}

/*!
    \relates QStringView
    \internal
    \since 5.10

    Returns an integer that compares to 0 as \a lhs compares to \a rhs.

    If \a cs is Qt::CaseSensitive (the default), the comparison is case-sensitive;
    otherwise the comparison is case-insensitive.

    Case-sensitive comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would expect.
    Consider sorting user-visible strings with QString::localeAwareCompare().

    \sa {Comparing Strings}
*/
int QtPrivate::compareStrings(QStringView lhs, QStringView rhs, Qt::CaseSensitivity cs) noexcept
{
    if (cs == Qt::CaseSensitive)
        return ucstrcmp(lhs.begin(), lhs.size(), rhs.begin(), rhs.size());
    return ucstricmp(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/*!
    \relates QStringView
    \internal
    \since 5.10
    \overload

    Returns an integer that compares to 0 as \a lhs compares to \a rhs.

    If \a cs is Qt::CaseSensitive (the default), the comparison is case-sensitive;
    otherwise the comparison is case-insensitive.

    Case-sensitive comparison is based exclusively on the numeric Unicode values
    of the characters and is very fast, but is not what a human would expect.
    Consider sorting user-visible strings with QString::localeAwareCompare().

    \sa {Comparing Strings}
*/
int QtPrivate::compareStrings(QStringView lhs, QLatin1String rhs, Qt::CaseSensitivity cs) noexcept
{
    if (cs == Qt::CaseSensitive)
        return ucstrcmp(lhs.begin(), lhs.size(), rhs.begin(), rhs.size());
    return ucstricmp(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/*!
    \relates QStringView
    \internal
    \since 6.0
    \overload
*/
int QtPrivate::compareStrings(QStringView lhs, QBasicUtf8StringView<false> rhs, Qt::CaseSensitivity cs) noexcept
{
    return -compareStrings(rhs, lhs, cs);
}

/*!
    \relates QStringView
    \internal
    \since 5.10
    \overload
*/
int QtPrivate::compareStrings(QLatin1String lhs, QStringView rhs, Qt::CaseSensitivity cs) noexcept
{
    return -compareStrings(rhs, lhs, cs);
}

/*!
    \relates QStringView
    \internal
    \since 5.10
    \overload

    Returns an integer that compares to 0 as \a lhs compares to \a rhs.

    If \a cs is Qt::CaseSensitive (the default), the comparison is case-sensitive;
    otherwise the comparison is case-insensitive.

    Case-sensitive comparison is based exclusively on the numeric Latin-1 values
    of the characters and is very fast, but is not what a human would expect.
    Consider sorting user-visible strings with QString::localeAwareCompare().

    \sa {Comparing Strings}
*/
int QtPrivate::compareStrings(QLatin1String lhs, QLatin1String rhs, Qt::CaseSensitivity cs) noexcept
{
    if (lhs.isEmpty())
        return lencmp(qsizetype(0), rhs.size());
    if (cs == Qt::CaseInsensitive)
        return latin1nicmp(lhs.data(), lhs.size(), rhs.data(), rhs.size());
    const auto l = std::min(lhs.size(), rhs.size());
    int r = qstrncmp(lhs.data(), rhs.data(), l);
    return r ? r : lencmp(lhs.size(), rhs.size());
}

/*!
    \relates QStringView
    \internal
    \since 6.0
    \overload
*/
int QtPrivate::compareStrings(QLatin1String lhs, QBasicUtf8StringView<false> rhs, Qt::CaseSensitivity cs) noexcept
{
    return compareStrings(lhs, rhs.toString(), cs); // ### optimize!
}

/*!
    \relates QStringView
    \internal
    \since 6.0
    \overload
*/
int QtPrivate::compareStrings(QBasicUtf8StringView<false> lhs, QStringView rhs, Qt::CaseSensitivity cs) noexcept
{
    if (cs == Qt::CaseSensitive)
        return QUtf8::compareUtf8(lhs, rhs);
    return ucstricmp8(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/*!
    \relates QStringView
    \internal
    \since 6.0
    \overload
*/
int QtPrivate::compareStrings(QBasicUtf8StringView<false> lhs, QLatin1String rhs, Qt::CaseSensitivity cs) noexcept
{
    return -compareStrings(rhs, lhs, cs);
}

/*!
    \relates QStringView
    \internal
    \since 6.0
    \overload
*/
int QtPrivate::compareStrings(QBasicUtf8StringView<false> lhs, QBasicUtf8StringView<false> rhs, Qt::CaseSensitivity cs) noexcept
{
    if (lhs.isEmpty())
        return lencmp(0, rhs.size());
    if (cs == Qt::CaseInsensitive)
        return compareStrings(lhs.toString(), rhs.toString(), cs); // ### optimize!
    const auto l = std::min(lhs.size(), rhs.size());
    int r = qstrncmp(lhs.data(), rhs.data(), l);
    return r ? r : lencmp(lhs.size(), rhs.size());
}

int QAnyStringView::compare(QAnyStringView lhs, QAnyStringView rhs, Qt::CaseSensitivity cs) noexcept
{
    return lhs.visit([rhs, cs](auto lhs) {
        return rhs.visit([lhs, cs](auto rhs) {
            return QtPrivate::compareStrings(lhs, rhs, cs);
        });
    });
}

#define REHASH(a) \
    if (sl_minus_1 < sizeof(std::size_t) * CHAR_BIT)  \
        hashHaystack -= std::size_t(a) << sl_minus_1; \
    hashHaystack <<= 1

inline bool qIsUpper(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

inline bool qIsDigit(char ch)
{
    return ch >= '0' && ch <= '9';
}

inline char qToLower(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return ch - 'A' + 'a';
    else
        return ch;
}


/*!
  \macro QT_RESTRICTED_CAST_FROM_ASCII
  \relates QString

  Disables most automatic conversions from source literals and 8-bit data
  to unicode QStrings, but allows the use of
  the \c{QChar(char)} and \c{QString(const char (&ch)[N]} constructors,
  and the \c{QString::operator=(const char (&ch)[N])} assignment operator.
  This gives most of the type-safety benefits of \l QT_NO_CAST_FROM_ASCII
  but does not require user code to wrap character and string literals
  with QLatin1Char, QLatin1String or similar.

  Using this macro together with source strings outside the 7-bit range,
  non-literals, or literals with embedded NUL characters is undefined.

  \sa QT_NO_CAST_FROM_ASCII, QT_NO_CAST_TO_ASCII
*/

/*!
  \macro QT_NO_CAST_FROM_ASCII
  \relates QString
  \relates QChar

  Disables automatic conversions from 8-bit strings (\c{char *}) to Unicode
  QStrings, as well as from 8-bit \c{char} types (\c{char} and
  \c{unsigned char}) to QChar.

  \sa QT_NO_CAST_TO_ASCII, QT_RESTRICTED_CAST_FROM_ASCII,
      QT_NO_CAST_FROM_BYTEARRAY
*/

/*!
  \macro QT_NO_CAST_TO_ASCII
  \relates QString

  Disables automatic conversion from QString to 8-bit strings (\c{char *}).

  \sa QT_NO_CAST_FROM_ASCII, QT_RESTRICTED_CAST_FROM_ASCII,
      QT_NO_CAST_FROM_BYTEARRAY
*/

/*!
  \macro QT_ASCII_CAST_WARNINGS
  \internal
  \relates QString

  This macro can be defined to force a warning whenever a function is
  called that automatically converts between unicode and 8-bit encodings.

  Note: This only works for compilers that support warnings for
  deprecated API.

  \sa QT_NO_CAST_TO_ASCII, QT_NO_CAST_FROM_ASCII, QT_RESTRICTED_CAST_FROM_ASCII
*/

/*!
    \class QString
    \inmodule QtCore
    \reentrant

    \brief The QString class provides a Unicode character string.

    \ingroup tools
    \ingroup shared
    \ingroup string-processing

    QString stores a string of 16-bit \l{QChar}s, where each QChar
    corresponds to one UTF-16 code unit. (Unicode characters
    with code values above 65535 are stored using surrogate pairs,
    i.e., two consecutive \l{QChar}s.)

    \l{Unicode} is an international standard that supports most of the
    writing systems in use today. It is a superset of US-ASCII (ANSI
    X3.4-1986) and Latin-1 (ISO 8859-1), and all the US-ASCII/Latin-1
    characters are available at the same code positions.

    Behind the scenes, QString uses \l{implicit sharing}
    (copy-on-write) to reduce memory usage and to avoid the needless
    copying of data. This also helps reduce the inherent overhead of
    storing 16-bit characters instead of 8-bit characters.

    In addition to QString, Qt also provides the QByteArray class to
    store raw bytes and traditional 8-bit '\\0'-terminated strings.
    For most purposes, QString is the class you want to use. It is
    used throughout the Qt API, and the Unicode support ensures that
    your applications will be easy to translate if you want to expand
    your application's market at some point. The two main cases where
    QByteArray is appropriate are when you need to store raw binary
    data, and when memory conservation is critical (like in embedded
    systems).

    \tableofcontents

    \section1 Initializing a String

    One way to initialize a QString is simply to pass a \c{const char
    *} to its constructor. For example, the following code creates a
    QString of size 5 containing the data "Hello":

    \snippet qstring/main.cpp 0

    QString converts the \c{const char *} data into Unicode using the
    fromUtf8() function.

    In all of the QString functions that take \c{const char *}
    parameters, the \c{const char *} is interpreted as a classic
    C-style '\\0'-terminated string encoded in UTF-8. It is legal for
    the \c{const char *} parameter to be \nullptr.

    You can also provide string data as an array of \l{QChar}s:

    \snippet qstring/main.cpp 1

    QString makes a deep copy of the QChar data, so you can modify it
    later without experiencing side effects. (If for performance
    reasons you don't want to take a deep copy of the character data,
    use QString::fromRawData() instead.)

    Another approach is to set the size of the string using resize()
    and to initialize the data character per character. QString uses
    0-based indexes, just like C++ arrays. To access the character at
    a particular index position, you can use \l operator[](). On
    non-\c{const} strings, \l operator[]() returns a reference to a
    character that can be used on the left side of an assignment. For
    example:

    \snippet qstring/main.cpp 2

    For read-only access, an alternative syntax is to use the at()
    function:

    \snippet qstring/main.cpp 3

    The at() function can be faster than \l operator[](), because it
    never causes a \l{deep copy} to occur. Alternatively, use the
    first(), last(), or sliced() functions to extract several characters
    at a time.

    A QString can embed '\\0' characters (QChar::Null). The size()
    function always returns the size of the whole string, including
    embedded '\\0' characters.

    After a call to the resize() function, newly allocated characters
    have undefined values. To set all the characters in the string to
    a particular value, use the fill() function.

    QString provides dozens of overloads designed to simplify string
    usage. For example, if you want to compare a QString with a string
    literal, you can write code like this and it will work as expected:

    \snippet qstring/main.cpp 4

    You can also pass string literals to functions that take QStrings
    as arguments, invoking the QString(const char *)
    constructor. Similarly, you can pass a QString to a function that
    takes a \c{const char *} argument using the \l qPrintable() macro
    which returns the given QString as a \c{const char *}. This is
    equivalent to calling <QString>.toLocal8Bit().constData().

    \section1 Manipulating String Data

    QString provides the following basic functions for modifying the
    character data: append(), prepend(), insert(), replace(), and
    remove(). For example:

    \snippet qstring/main.cpp 5

    In the above example the replace() function's first two arguments are the
    position from which to start replacing and the number of characters that
    should be replaced.

    When data-modifying functions increase the size of the string,
    they may lead to reallocation of memory for the QString object. When
    this happens, QString expands by more than it immediately needs so as
    to have space for further expansion without reallocation until the size
    of the string has greatly increased.

    The insert(), remove() and, when replacing a sub-string with one of
    different size, replace() functions can be slow (\l{linear time}) for
    large strings, because they require moving many characters in the string
    by at least one position in memory.

    If you are building a QString gradually and know in advance
    approximately how many characters the QString will contain, you
    can call reserve(), asking QString to preallocate a certain amount
    of memory. You can also call capacity() to find out how much
    memory the QString actually has allocated.

    QString provides \l{STL-style iterators} (QString::const_iterator and
    QString::iterator). In practice, iterators are handy when working with
    generic algorithms provided by the C++ standard library.

    \note Iterators over a QString, and references to individual characters
    within one, cannot be relied on to remain valid when any non-\c{const}
    method of the QString is called. Accessing such an iterator or reference
    after the call to a non-\c{const} method leads to undefined behavior. When
    stability for iterator-like functionality is required, you should use
    indexes instead of iterators as they are not tied to QString's internal
    state and thus do not get invalidated.

    \note Due to \l{implicit sharing}, the first non-\c{const} operator or
    function used on a given QString may cause it to, internally, perform a deep
    copy of its data. This invalidates all iterators over the string and
    references to individual characters within it. After the first non-\c{const}
    operator, operations that modify QString may completely (in case of
    reallocation) or partially invalidate iterators and references, but other
    methods (such as begin() or end()) will not. Accessing an iterator or
    reference after it has been invalidated leads to undefined behavior.

    A frequent requirement is to remove whitespace characters from a
    string ('\\n', '\\t', ' ', etc.). If you want to remove whitespace
    from both ends of a QString, use the trimmed() function. If you
    want to remove whitespace from both ends and replace multiple
    consecutive whitespaces with a single space character within the
    string, use simplified().

    If you want to find all occurrences of a particular character or
    substring in a QString, use the indexOf() or lastIndexOf()
    functions. The former searches forward starting from a given index
    position, the latter searches backward. Both return the index
    position of the character or substring if they find it; otherwise,
    they return -1.  For example, here is a typical loop that finds all
    occurrences of a particular substring:

    \snippet qstring/main.cpp 6

    QString provides many functions for converting numbers into
    strings and strings into numbers. See the arg() functions, the
    setNum() functions, the number() static functions, and the
    toInt(), toDouble(), and similar functions.

    To get an upper- or lowercase version of a string use toUpper() or
    toLower().

    Lists of strings are handled by the QStringList class. You can
    split a string into a list of strings using the split() function,
    and join a list of strings into a single string with an optional
    separator using QStringList::join(). You can obtain a list of
    strings from a string list that contain a particular substring or
    that match a particular QRegularExpression using the QStringList::filter()
    function.

    \section1 Querying String Data

    If you want to see if a QString starts or ends with a particular
    substring use startsWith() or endsWith(). If you simply want to
    check whether a QString contains a particular character or
    substring, use the contains() function. If you want to find out
    how many times a particular character or substring occurs in the
    string, use count().

    To obtain a pointer to the actual character data, call data() or
    constData(). These functions return a pointer to the beginning of
    the QChar data. The pointer is guaranteed to remain valid until a
    non-\c{const} function is called on the QString.

    \section2 Comparing Strings

    QStrings can be compared using overloaded operators such as \l
    operator<(), \l operator<=(), \l operator==(), \l operator>=(),
    and so on.  Note that the comparison is based exclusively on the
    numeric Unicode values of the characters. It is very fast, but is
    not what a human would expect; the QString::localeAwareCompare()
    function is usually a better choice for sorting user-interface
    strings, when such a comparison is available.

    On Unix-like platforms (including Linux, \macos and iOS), when Qt
    is linked with the ICU library (which it usually is), its
    locale-aware sorting is used.  Otherwise, on \macos and iOS, \l
    localeAwareCompare() compares according the "Order for sorted
    lists" setting in the International preferences panel. On other
    Unix-like systems without ICU, the comparison falls back to the
    system library's \c strcoll(),

    \section1 Converting Between Encoded Strings Data and QString

    QString provides the following three functions that return a
    \c{const char *} version of the string as QByteArray: toUtf8(),
    toLatin1(), and toLocal8Bit().

    \list
    \li toLatin1() returns a Latin-1 (ISO 8859-1) encoded 8-bit string.
    \li toUtf8() returns a UTF-8 encoded 8-bit string. UTF-8 is a
       superset of US-ASCII (ANSI X3.4-1986) that supports the entire
       Unicode character set through multibyte sequences.
    \li toLocal8Bit() returns an 8-bit string using the system's local
       encoding. This is the same as toUtf8() on Unix systems.
    \endlist

    To convert from one of these encodings, QString provides
    fromLatin1(), fromUtf8(), and fromLocal8Bit(). Other
    encodings are supported through the QStringEncoder and QStringDecoder
    classes.

    As mentioned above, QString provides a lot of functions and
    operators that make it easy to interoperate with \c{const char *}
    strings. But this functionality is a double-edged sword: It makes
    QString more convenient to use if all strings are US-ASCII or
    Latin-1, but there is always the risk that an implicit conversion
    from or to \c{const char *} is done using the wrong 8-bit
    encoding. To minimize these risks, you can turn off these implicit
    conversions by defining some of the following preprocessor symbols:

    \list
    \li \l QT_NO_CAST_FROM_ASCII disables automatic conversions from
       C string literals and pointers to Unicode.
    \li \l QT_RESTRICTED_CAST_FROM_ASCII allows automatic conversions
       from C characters and character arrays, but disables automatic
       conversions from character pointers to Unicode.
    \li \l QT_NO_CAST_TO_ASCII disables automatic conversion from QString
       to C strings.
    \endlist

    You then need to explicitly call fromUtf8(), fromLatin1(),
    or fromLocal8Bit() to construct a QString from an
    8-bit string, or use the lightweight QLatin1String class, for
    example:

    \snippet code/src_corelib_text_qstring.cpp 1

    Similarly, you must call toLatin1(), toUtf8(), or
    toLocal8Bit() explicitly to convert the QString to an 8-bit
    string.

    \table 100 %
    \header
    \li Note for C Programmers

    \row
    \li
    Due to C++'s type system and the fact that QString is
    \l{implicitly shared}, QStrings may be treated like \c{int}s or
    other basic types. For example:

    \snippet qstring/main.cpp 7

    The \c result variable, is a normal variable allocated on the
    stack. When \c return is called, and because we're returning by
    value, the copy constructor is called and a copy of the string is
    returned. No actual copying takes place thanks to the implicit
    sharing.

    \endtable

    \section1 Distinction Between Null and Empty Strings

    For historical reasons, QString distinguishes between a null
    string and an empty string. A \e null string is a string that is
    initialized using QString's default constructor or by passing
    (\c{const char *})0 to the constructor. An \e empty string is any
    string with size 0. A null string is always empty, but an empty
    string isn't necessarily null:

    \snippet qstring/main.cpp 8

    All functions except isNull() treat null strings the same as empty
    strings. For example, toUtf8().constData() returns a valid pointer
    (\e not nullptr) to a '\\0' character for a null string. We
    recommend that you always use the isEmpty() function and avoid isNull().

    \section1 Number Formats

    When a QString::arg() \c{'%'} format specifier includes the \c{'L'} locale
    qualifier, and the base is ten (its default), the default locale is
    used. This can be set using \l{QLocale::setDefault()}. For more refined
    control of localized string representations of numbers, see
    QLocale::toString(). All other number formatting done by QString follows the
    C locale's representation of numbers.

    When QString::arg() applies left-padding to numbers, the fill character
    \c{'0'} is treated specially. If the number is negative, its minus sign will
    appear before the zero-padding. If the field is localized, the
    locale-appropriate zero character is used in place of \c{'0'}. For
    floating-point numbers, this special treatment only applies if the number is
    finite.

    \section2 Floating-point Formats

    In member functions (e.g., arg(), number()) that represent floating-point
    numbers (\c float or \c double) as strings, the form of display can be
    controlled by a choice of \e format and \e precision, whose meanings are as
    for \l {QLocale::toString(double, char, int)}.

    If the selected \e format includes an exponent, localized forms follow the
    locale's convention on digits in the exponent. For non-localized formatting,
    the exponent shows its sign and includes at least two digits, left-padding
    with zero if needed.

    \section1 More Efficient String Construction

    Many strings are known at compile time. But the trivial
    constructor QString("Hello"), will copy the contents of the string,
    treating the contents as Latin-1. To avoid this one can use the
    QStringLiteral macro to directly create the required data at compile
    time. Constructing a QString out of the literal does then not cause
    any overhead at runtime.

    A slightly less efficient way is to use QLatin1String. This class wraps
    a C string literal, precalculates it length at compile time and can
    then be used for faster comparison with QStrings and conversion to
    QStrings than a regular C string literal.

    Using the QString \c{'+'} operator, it is easy to construct a
    complex string from multiple substrings. You will often write code
    like this:

    \snippet qstring/stringbuilder.cpp 0

    There is nothing wrong with either of these string constructions,
    but there are a few hidden inefficiencies. Beginning with Qt 4.6,
    you can eliminate them.

    First, multiple uses of the \c{'+'} operator usually means
    multiple memory allocations. When concatenating \e{n} substrings,
    where \e{n > 2}, there can be as many as \e{n - 1} calls to the
    memory allocator.

    In 4.6, an internal template class \c{QStringBuilder} has been
    added along with a few helper functions. This class is marked
    internal and does not appear in the documentation, because you
    aren't meant to instantiate it in your code. Its use will be
    automatic, as described below. The class is found in
    \c {src/corelib/tools/qstringbuilder.cpp} if you want to have a
    look at it.

    \c{QStringBuilder} uses expression templates and reimplements the
    \c{'%'} operator so that when you use \c{'%'} for string
    concatenation instead of \c{'+'}, multiple substring
    concatenations will be postponed until the final result is about
    to be assigned to a QString. At this point, the amount of memory
    required for the final result is known. The memory allocator is
    then called \e{once} to get the required space, and the substrings
    are copied into it one by one.

    Additional efficiency is gained by inlining and reduced reference
    counting (the QString created from a \c{QStringBuilder} typically
    has a ref count of 1, whereas QString::append() needs an extra
    test).

    There are two ways you can access this improved method of string
    construction. The straightforward way is to include
    \c{QStringBuilder} wherever you want to use it, and use the
    \c{'%'} operator instead of \c{'+'} when concatenating strings:

    \snippet qstring/stringbuilder.cpp 5

    A more global approach which is the most convenient but
    not entirely source compatible, is to this define in your
    .pro file:

    \snippet qstring/stringbuilder.cpp 3

    and the \c{'+'} will automatically be performed as the
    \c{QStringBuilder} \c{'%'} everywhere.

    \section1 Maximum Size and Out-of-memory Conditions

    The maximum size of QString depends on the architecture. Most 64-bit
    systems can allocate more than 2 GB of memory, with a typical limit
    of 2^63 bytes. The actual value also depends on the overhead required for
    managing the data block. As a result, you can expect the maximum size
    of 2 GB minus overhead on 32-bit platforms, and 2^63 bytes minus overhead
    on 64-bit platforms. The number of elements that can be stored in a
    QString is this maximum size divided by the size of QChar.

    When memory allocation fails, QString throws a \c std::bad_alloc
    exception if the application was compiled with exception support.
    Out of memory conditions in Qt containers are the only case where Qt
    will throw exceptions. If exceptions are disabled, then running out of
    memory is undefined behavior.

    Note that the operating system may impose further limits on applications
    holding a lot of allocated memory, especially large, contiguous blocks.
    Such considerations, the configuration of such behavior or any mitigation
    are outside the scope of the Qt API.

    \sa fromRawData(), QChar, QStringView, QLatin1String, QByteArray
*/

/*! \typedef QString::ConstIterator

    Qt-style synonym for QString::const_iterator.
*/

/*! \typedef QString::Iterator

    Qt-style synonym for QString::iterator.
*/

/*! \typedef QString::const_iterator

    \sa QString::iterator
*/

/*! \typedef QString::iterator

    \sa QString::const_iterator
*/

/*! \typedef QString::const_reverse_iterator
    \since 5.6

    \sa QString::reverse_iterator, QString::const_iterator
*/

/*! \typedef QString::reverse_iterator
    \since 5.6

    \sa QString::const_reverse_iterator, QString::iterator
*/

/*!
    \typedef QString::size_type
*/

/*!
    \typedef QString::difference_type
*/

/*!
    \typedef QString::const_reference
*/
/*!
    \typedef QString::reference
*/

/*!
    \typedef QString::const_pointer

    The QString::const_pointer typedef provides an STL-style
    const pointer to a QString element (QChar).
*/
/*!
    \typedef QString::pointer

    The QString::pointer typedef provides an STL-style
    pointer to a QString element (QChar).
*/

/*!
    \typedef QString::value_type
*/

/*! \fn QString::iterator QString::begin()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing to the
    first character in the string.

//! [iterator-invalidation-func-desc]
    \warning The returned iterator is invalidated on detachment or when the
    QString is modified.
//! [iterator-invalidation-func-desc]

    \sa constBegin(), end()
*/

/*! \fn QString::const_iterator QString::begin() const

    \overload begin()
*/

/*! \fn QString::const_iterator QString::cbegin() const
    \since 5.0

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the
    first character in the string.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa begin(), cend()
*/

/*! \fn QString::const_iterator QString::constBegin() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the
    first character in the string.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa begin(), constEnd()
*/

/*! \fn QString::iterator QString::end()

    Returns an \l{STL-style iterators}{STL-style iterator} pointing just after
    the last character in the string.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa begin(), constEnd()
*/

/*! \fn QString::const_iterator QString::end() const

    \overload end()
*/

/*! \fn QString::const_iterator QString::cend() const
    \since 5.0

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing just
    after the last character in the string.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa cbegin(), end()
*/

/*! \fn QString::const_iterator QString::constEnd() const

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing just
    after the last character in the string.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa constBegin(), end()
*/

/*! \fn QString::reverse_iterator QString::rbegin()
    \since 5.6

    Returns a \l{STL-style iterators}{STL-style} reverse iterator pointing to
    the first character in the string, in reverse order.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa begin(), crbegin(), rend()
*/

/*! \fn QString::const_reverse_iterator QString::rbegin() const
    \since 5.6
    \overload
*/

/*! \fn QString::const_reverse_iterator QString::crbegin() const
    \since 5.6

    Returns a const \l{STL-style iterators}{STL-style} reverse iterator
    pointing to the first character in the string, in reverse order.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa begin(), rbegin(), rend()
*/

/*! \fn QString::reverse_iterator QString::rend()
    \since 5.6

    Returns a \l{STL-style iterators}{STL-style} reverse iterator pointing just
    after the last character in the string, in reverse order.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa end(), crend(), rbegin()
*/

/*! \fn QString::const_reverse_iterator QString::rend() const
    \since 5.6
    \overload
*/

/*! \fn QString::const_reverse_iterator QString::crend() const
    \since 5.6

    Returns a const \l{STL-style iterators}{STL-style} reverse iterator
    pointing just after the last character in the string, in reverse order.

    \include qstring.cpp iterator-invalidation-func-desc

    \sa end(), rend(), rbegin()
*/

/*!
    \fn QString::QString()

    Constructs a null string. Null strings are also empty.

    \sa isEmpty()
*/

/*!
    \fn QString::QString(QString &&other)

    Move-constructs a QString instance, making it point at the same
    object that \a other was pointing to.

    \since 5.2
*/

/*! \fn QString::QString(const char *str)

    Constructs a string initialized with the 8-bit string \a str. The
    given const char pointer is converted to Unicode using the
    fromUtf8() function.

    You can disable this constructor by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \note Defining \l QT_RESTRICTED_CAST_FROM_ASCII also disables
    this constructor, but enables a \c{QString(const char (&ch)[N])}
    constructor instead. Using non-literal input, or input with
    embedded NUL characters, or non-7-bit characters is undefined
    in this case.

    \sa fromLatin1(), fromLocal8Bit(), fromUtf8()
*/

/*! \fn QString::QString(const char8_t *str)

    Constructs a string initialized with the UTF-8 string \a str. The
    given const char8_t pointer is converted to Unicode using the
    fromUtf8() function.

    \since 6.1
    \sa fromLatin1(), fromLocal8Bit(), fromUtf8()
*/

/*! \fn QString QString::fromStdString(const std::string &str)

    Returns a copy of the \a str string. The given string is converted
    to Unicode using the fromUtf8() function.

    \sa fromLatin1(), fromLocal8Bit(), fromUtf8(), QByteArray::fromStdString()
*/

/*! \fn QString QString::fromStdWString(const std::wstring &str)

    Returns a copy of the \a str string. The given string is assumed
    to be encoded in utf16 if the size of wchar_t is 2 bytes (e.g. on
    windows) and ucs4 if the size of wchar_t is 4 bytes (most Unix
    systems).

    \sa fromUtf16(), fromLatin1(), fromLocal8Bit(), fromUtf8(), fromUcs4(),
        fromStdU16String(), fromStdU32String()
*/

/*! \fn QString QString::fromWCharArray(const wchar_t *string, qsizetype size)
    \since 4.2

    Returns a copy of the \a string, where the encoding of \a string depends on
    the size of wchar. If wchar is 4 bytes, the \a string is interpreted as
    UCS-4, if wchar is 2 bytes it is interpreted as UTF-16.

    If \a size is -1 (default), the \a string must be '\\0'-terminated.

    \sa fromUtf16(), fromLatin1(), fromLocal8Bit(), fromUtf8(), fromUcs4(),
        fromStdWString()
*/

/*! \fn std::wstring QString::toStdWString() const

    Returns a std::wstring object with the data contained in this
    QString. The std::wstring is encoded in utf16 on platforms where
    wchar_t is 2 bytes wide (e.g. windows) and in ucs4 on platforms
    where wchar_t is 4 bytes wide (most Unix systems).

    This method is mostly useful to pass a QString to a function
    that accepts a std::wstring object.

    \sa utf16(), toLatin1(), toUtf8(), toLocal8Bit(), toStdU16String(),
        toStdU32String()
*/

qsizetype QString::toUcs4_helper(const ushort *uc, qsizetype length, uint *out)
{
    qsizetype count = 0;

    QStringIterator i(QStringView(uc, length));
    while (i.hasNext())
        out[count++] = i.next();

    return count;
}

/*! \fn qsizetype QString::toWCharArray(wchar_t *array) const
  \since 4.2

  Fills the \a array with the data contained in this QString object.
  The array is encoded in UTF-16 on platforms where
  wchar_t is 2 bytes wide (e.g. windows) and in UCS-4 on platforms
  where wchar_t is 4 bytes wide (most Unix systems).

  \a array has to be allocated by the caller and contain enough space to
  hold the complete string (allocating the array with the same length as the
  string is always sufficient).

  This function returns the actual length of the string in \a array.

  \note This function does not append a null character to the array.

  \sa utf16(), toUcs4(), toLatin1(), toUtf8(), toLocal8Bit(), toStdWString(),
      QStringView::toWCharArray()
*/

/*! \fn QString::QString(const QString &other)

    Constructs a copy of \a other.

    This operation takes \l{constant time}, because QString is
    \l{implicitly shared}. This makes returning a QString from a
    function very fast. If a shared instance is modified, it will be
    copied (copy-on-write), and that takes \l{linear time}.

    \sa operator=()
*/

/*!
    Constructs a string initialized with the first \a size characters
    of the QChar array \a unicode.

    If \a unicode is 0, a null string is constructed.

    If \a size is negative, \a unicode is assumed to point to a \\0'-terminated
    array and its length is determined dynamically. The terminating
    null character is not considered part of the string.

    QString makes a deep copy of the string data. The unicode data is copied as
    is and the Byte Order Mark is preserved if present.

    \sa fromRawData()
*/
QString::QString(const QChar *unicode, qsizetype size)
{
    if (!unicode) {
        d.clear();
    } else {
        if (size < 0) {
            size = 0;
            while (!unicode[size].isNull())
                ++size;
        }
        if (!size) {
            d = DataPointer::fromRawData(&_empty, 0);
        } else {
            d = DataPointer(Data::allocate(size), size);
            Q_CHECK_PTR(d.data());
            memcpy(d.data(), unicode, size * sizeof(QChar));
            d.data()[size] = '\0';
        }
    }
}

/*!
    Constructs a string of the given \a size with every character set
    to \a ch.

    \sa fill()
*/
QString::QString(qsizetype size, QChar ch)
{
    if (size <= 0) {
        d = DataPointer::fromRawData(&_empty, 0);
    } else {
        d = DataPointer(Data::allocate(size), size);
        Q_CHECK_PTR(d.data());
        d.data()[size] = '\0';
        char16_t *i = d.data() + size;
        char16_t *b = d.data();
        const char16_t value = ch.unicode();
        while (i != b)
           *--i = value;
    }
}

/*! \fn QString::QString(qsizetype size, Qt::Initialization)
  \internal

  Constructs a string of the given \a size without initializing the
  characters. This is only used in \c QStringBuilder::toString().
*/
QString::QString(qsizetype size, Qt::Initialization)
{
    if (size <= 0) {
        d = DataPointer::fromRawData(&_empty, 0);
    } else {
        d = DataPointer(Data::allocate(size), size);
        Q_CHECK_PTR(d.data());
        d.data()[size] = '\0';
    }
}

/*! \fn QString::QString(QLatin1String str)

    Constructs a copy of the Latin-1 string \a str.

    \sa fromLatin1()
*/

/*!
    Constructs a string of size 1 containing the character \a ch.
*/
QString::QString(QChar ch)
{
    d = DataPointer(Data::allocate(1), 1);
    Q_CHECK_PTR(d.data());
    d.data()[0] = ch.unicode();
    d.data()[1] = '\0';
}

/*! \fn QString::QString(const QByteArray &ba)

    Constructs a string initialized with the byte array \a ba. The
    given byte array is converted to Unicode using fromUtf8().

    You can disable this constructor by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000). This behavior is
    different from Qt 5.x.

    \sa fromLatin1(), fromLocal8Bit(), fromUtf8()
*/

/*! \fn QString::QString(const Null &)
    \internal
*/

/*! \fn QString::QString(QStringPrivate)
    \internal
*/

/*! \fn QString &QString::operator=(const QString::Null &)
    \internal
*/

/*!
  \fn QString::~QString()

    Destroys the string.
*/


/*! \fn void QString::swap(QString &other)
    \since 4.8

    Swaps string \a other with this string. This operation is very fast and
    never fails.
*/

/*! \fn void QString::detach()

    \internal
*/

/*! \fn bool QString::isDetached() const

    \internal
*/

/*! \fn bool QString::isSharedWith(const QString &other) const

    \internal
*/

/*!
    Sets the size of the string to \a size characters.

    If \a size is greater than the current size, the string is
    extended to make it \a size characters long with the extra
    characters added to the end. The new characters are uninitialized.

    If \a size is less than the current size, characters beyond position
    \a size are excluded from the string.

    \note While resize() will grow the capacity if needed, it never shrinks
    capacity. To shed excess capacity, use squeeze().

    Example:

    \snippet qstring/main.cpp 45

    If you want to append a certain number of identical characters to
    the string, use the \l {QString::}{resize(qsizetype, QChar)} overload.

    If you want to expand the string so that it reaches a certain
    width and fill the new positions with a particular character, use
    the leftJustified() function:

    If \a size is negative, it is equivalent to passing zero.

    \snippet qstring/main.cpp 47

    \sa truncate(), reserve(), squeeze()
*/

void QString::resize(qsizetype size)
{
    if (size < 0)
        size = 0;

    const auto capacityAtEnd = capacity() - d.freeSpaceAtBegin();
    if (d->needsDetach() || size > capacityAtEnd)
        reallocData(size, QArrayData::Grow);
    d.size = size;
    if (d->allocatedCapacity())
        d.data()[size] = 0;
}

/*!
    \overload
    \since 5.7

    Unlike \l {QString::}{resize(qsizetype)}, this overload
    initializes the new characters to \a fillChar:

    \snippet qstring/main.cpp 46
*/

void QString::resize(qsizetype size, QChar fillChar)
{
    const qsizetype oldSize = length();
    resize(size);
    const qsizetype difference = length() - oldSize;
    if (difference > 0)
        std::fill_n(d.data() + oldSize, difference, fillChar.unicode());
}

/*! \fn qsizetype QString::capacity() const

    Returns the maximum number of characters that can be stored in
    the string without forcing a reallocation.

    The sole purpose of this function is to provide a means of fine
    tuning QString's memory usage. In general, you will rarely ever
    need to call this function. If you want to know how many
    characters are in the string, call size().

    \note a statically allocated string will report a capacity of 0,
    even if it's not empty.

    \note The free space position in the allocated memory block is undefined. In
    other words, one should not assume that the free memory is always located
    after the initialized elements.

    \sa reserve(), squeeze()
*/

/*!
    \fn void QString::reserve(qsizetype size)

    Ensures the string has space for at least \a size characters.

    If you know in advance how large the string will be, you can call this
    function to save repeated reallocation in the course of building it.
    This can improve performance when building a string incrementally.
    A long sequence of operations that add to a string may trigger several
    reallocations, the last of which may leave you with significantly more
    space than you really need, which is less efficient than doing a single
    allocation of the right size at the start.

    If in doubt about how much space shall be needed, it is usually better to
    use an upper bound as \a size, or a high estimate of the most likely size,
    if a strict upper bound would be much bigger than this. If \a size is an
    underestimate, the string will grow as needed once the reserved size is
    exceeded, which may lead to a larger allocation than your best overestimate
    would have and will slow the operation that triggers it.

    \warning reserve() reserves memory but does not change the size of the
    string. Accessing data beyond the end of the string is undefined behavior.
    If you need to access memory beyond the current end of the string,
    use resize().

    This function is useful for code that needs to build up a long
    string and wants to avoid repeated reallocation. In this example,
    we want to add to the string until some condition is \c true, and
    we're fairly sure that size is large enough to make a call to
    reserve() worthwhile:

    \snippet qstring/main.cpp 44

    \sa squeeze(), capacity(), resize()
*/

/*!
    \fn void QString::squeeze()

    Releases any memory not required to store the character data.

    The sole purpose of this function is to provide a means of fine
    tuning QString's memory usage. In general, you will rarely ever
    need to call this function.

    \sa reserve(), capacity()
*/

void QString::reallocData(qsizetype alloc, QArrayData::AllocationOption option)
{
    if (!alloc) {
        d = DataPointer::fromRawData(&_empty, 0);
        return;
    }

    // don't use reallocate path when reducing capacity and there's free space
    // at the beginning: might shift data pointer outside of allocated space
    const bool cannotUseReallocate = d.freeSpaceAtBegin() > 0;

    if (d->needsDetach() || cannotUseReallocate) {
        DataPointer dd(Data::allocate(alloc, option), qMin(alloc, d.size));
        Q_CHECK_PTR(dd.data());
        if (dd.size > 0)
            ::memcpy(dd.data(), d.data(), dd.size * sizeof(QChar));
        dd.data()[dd.size] = 0;
        d = dd;
    } else {
        d->reallocate(alloc, option);
    }
}

void QString::reallocGrowData(qsizetype n)
{
    if (!n)  // expected to always allocate
        n = 1;

    if (d->needsDetach()) {
        DataPointer dd(DataPointer::allocateGrow(d, n, QArrayData::GrowsAtEnd));
        Q_CHECK_PTR(dd.data());
        dd->copyAppend(d.data(), d.data() + d.size);
        dd.data()[dd.size] = 0;
        d = dd;
    } else {
        d->reallocate(d.constAllocatedCapacity() + n, QArrayData::Grow);
    }
}

/*! \fn void QString::clear()

    Clears the contents of the string and makes it null.

    \sa resize(), isNull()
*/

/*! \fn QString &QString::operator=(const QString &other)

    Assigns \a other to this string and returns a reference to this
    string.
*/

QString &QString::operator=(const QString &other) noexcept
{
    d = other.d;
    return *this;
}

/*!
    \fn QString &QString::operator=(QString &&other)

    Move-assigns \a other to this QString instance.

    \since 5.2
*/

/*! \fn QString &QString::operator=(QLatin1String str)

    \overload operator=()

    Assigns the Latin-1 string \a str to this string.
*/
QString &QString::operator=(QLatin1String other)
{
    const qsizetype capacityAtEnd = capacity() - d.freeSpaceAtBegin();
    if (isDetached() && other.size() <= capacityAtEnd) { // assumes d->alloc == 0 -> !isDetached() (sharedNull)
        d.size = other.size();
        d.data()[other.size()] = 0;
        qt_from_latin1(d.data(), other.latin1(), other.size());
    } else {
        *this = fromLatin1(other.latin1(), other.size());
    }
    return *this;
}

/*! \fn QString &QString::operator=(const QByteArray &ba)

    \overload operator=()

    Assigns \a ba to this string. The byte array is converted to Unicode
    using the fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn QString &QString::operator=(const char *str)

    \overload operator=()

    Assigns \a str to this string. The const char pointer is converted
    to Unicode using the fromUtf8() function.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    or \l QT_RESTRICTED_CAST_FROM_ASCII when you compile your applications.
    This can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \overload operator=()

    Sets the string to contain the single character \a ch.
*/
QString &QString::operator=(QChar ch)
{
    const qsizetype capacityAtEnd = capacity() - d.freeSpaceAtBegin();
    if (isDetached() && capacityAtEnd >= 1) { // assumes d->alloc == 0 -> !isDetached() (sharedNull)
        // re-use existing capacity:
        d.data()[0] = ch.unicode();
        d.data()[1] = 0;
        d.size = 1;
    } else {
        operator=(QString(ch));
    }
    return *this;
}

/*!
     \fn QString& QString::insert(qsizetype position, const QString &str)

    Inserts the string \a str at the given index \a position and
    returns a reference to this string.

    Example:

    \snippet qstring/main.cpp 26

//! [string-grow-at-insertion]
    This string grows to accommodate the insertion. If \a position is beyond
    the end of the string, space characters are appended to the string to reach
    this \a position, followed by \a str.
//! [string-grow-at-insertion]

    \sa append(), prepend(), replace(), remove()
*/

/*!
    \fn QString& QString::insert(qsizetype position, QStringView str)
    \since 6.0
    \overload insert()

    Inserts the string view \a str at the given index \a position and
    returns a reference to this string.

    \include qstring.cpp string-grow-at-insertion
*/


/*!
    \fn QString& QString::insert(qsizetype position, const char *str)
    \since 5.5
    \overload insert()

    Inserts the C string \a str at the given index \a position and
    returns a reference to this string.

    \include qstring.cpp string-grow-at-insertion

    This function is not available when \l QT_NO_CAST_FROM_ASCII is
    defined.
*/


/*!
    \fn QString& QString::insert(qsizetype position, const QByteArray &str)
    \since 5.5
    \overload insert()

    Interprets the contents of \a str as UTF-8, inserts the Unicode string
    it encodes at the given index \a position and returns a reference to
    this string.

    \include qstring.cpp string-grow-at-insertion

    This function is not available when \l QT_NO_CAST_FROM_ASCII is
    defined.
*/

/*!
    \fn QString &QString::insert(qsizetype position, QLatin1String str)
    \overload insert()

    Inserts the Latin-1 string \a str at the given index \a position.

    \include qstring.cpp string-grow-at-insertion
*/
QString &QString::insert(qsizetype i, QLatin1String str)
{
    const char *s = str.latin1();
    if (i < 0 || !s || !(*s))
        return *this;

    qsizetype len = str.size();
    qsizetype difference = 0;
    if (Q_UNLIKELY(i > size()))
        difference = i - size();
    d.detachAndGrow(Data::GrowsAtEnd, difference + len, nullptr, nullptr);
    Q_CHECK_PTR(d.data());
    d->copyAppend(difference, u' ');
    d.size += len;

    ::memmove(d.data() + i + len, d.data() + i, (d.size - i - len) * sizeof(QChar));
    qt_from_latin1(d.data() + i, s, size_t(len));
    d.data()[d.size] = u'\0';
    return *this;
}

/*!
    \fn QString& QString::insert(qsizetype position, const QChar *unicode, qsizetype size)
    \overload insert()

    Inserts the first \a size characters of the QChar array \a unicode
    at the given index \a position in the string.

    This string grows to accommodate the insertion. If \a position is beyond
    the end of the string, space characters are appended to the string to reach
    this \a position, followed by \a size characters of the QChar array
    \a unicode.
*/
QString& QString::insert(qsizetype i, const QChar *unicode, qsizetype size)
{
    if (i < 0 || size <= 0)
        return *this;

    const char16_t *s = reinterpret_cast<const char16_t *>(unicode);

    // handle this specially, as QArrayDataOps::insert() doesn't handle out of
    // bounds positions
    if (i >= d->size) {
        // In case when data points into the range or is == *this, we need to
        // defer a call to free() so that it comes after we copied the data from
        // the old memory:
        DataPointer detached{};  // construction is free
        d.detachAndGrow(Data::GrowsAtEnd, (i - d.size) + size, &s, &detached);
        Q_CHECK_PTR(d.data());
        d->copyAppend(i - d->size, u' ');
        d->copyAppend(s, s + size);
        d.data()[d.size] = u'\0';
        return *this;
    }

    if (!d->needsDetach() && QtPrivate::q_points_into_range(s, d.data(), d.data() + d.size))
        return insert(i, QStringView{QVarLengthArray(s, s + size)});

    d->insert(i, s, size);
    d.data()[d.size] = u'\0';
    return *this;
}

/*!
    \fn QString& QString::insert(qsizetype position, QChar ch)
    \overload insert()

    Inserts \a ch at the given index \a position in the string.

    This string grows to accommodate the insertion. If \a position is beyond
    the end of the string, space characters are appended to the string to reach
    this \a position, followed by \a ch.
*/

QString& QString::insert(qsizetype i, QChar ch)
{
    if (i < 0)
        i += d.size;
    return insert(i, &ch, 1);
}

/*!
    Appends the string \a str onto the end of this string.

    Example:

    \snippet qstring/main.cpp 9

    This is the same as using the insert() function:

    \snippet qstring/main.cpp 10

    The append() function is typically very fast (\l{constant time}),
    because QString preallocates extra space at the end of the string
    data so it can grow without reallocating the entire string each
    time.

    \sa operator+=(), prepend(), insert()
*/
QString &QString::append(const QString &str)
{
    if (!str.isNull()) {
        if (isNull()) {
            operator=(str);
            if (Q_UNLIKELY(!d.isMutable() && d.size > 0)) {
                d.detach(); // fromRawData, so we do a deep copy
                d.data()[d.size] = u'\0';
            }
        } else if (str.size()) {
            append(str.constData(), str.size());
        }
    }
    return *this;
}

/*!
    \fn QString &QString::append(QStringView v)
    \overload append()
    \since 6.0

    Appends the given string view \a v to this string and returns the result.
*/

/*!
  \overload append()
  \since 5.0

  Appends \a len characters from the QChar array \a str to this string.
*/
QString &QString::append(const QChar *str, qsizetype len)
{
    if (str && len > 0) {
        static_assert(sizeof(QChar) == sizeof(char16_t), "Unexpected difference in sizes");
        // the following should be safe as QChar uses char16_t as underlying data
        const char16_t *char16String = reinterpret_cast<const char16_t *>(str);
        d->growAppend(char16String, char16String + len);
        d.data()[d.size] = u'\0';
    }
    return *this;
}

/*!
  \overload append()

  Appends the Latin-1 string \a str to this string.
*/
QString &QString::append(QLatin1String str)
{
    const char *s = str.latin1();
    const qsizetype len = str.size();
    if (s && len > 0) {
        d.detachAndGrow(Data::GrowsAtEnd, len, nullptr, nullptr);
        Q_CHECK_PTR(d.data());
        Q_ASSERT(len <= d->freeSpaceAtEnd());
        char16_t *i = d.data() + d.size;
        qt_from_latin1(i, s, size_t(len));
        d.size += len;
        d.data()[d.size] = '\0';
    } else if (d.isNull() && !str.isNull()) { // special case
        d = DataPointer::fromRawData(&_empty, 0);
    }
    return *this;
}

/*! \fn QString &QString::append(const QByteArray &ba)

    \overload append()

    Appends the byte array \a ba to this string. The given byte array
    is converted to Unicode using the fromUtf8() function.

    You can disable this function by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn QString &QString::append(const char *str)

    \overload append()

    Appends the string \a str to this string. The given const char
    pointer is converted to Unicode using the fromUtf8() function.

    You can disable this function by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*!
    \overload append()

    Appends the character \a ch to this string.
*/
QString &QString::append(QChar ch)
{
    d.detachAndGrow(QArrayData::GrowsAtEnd, 1, nullptr, nullptr);
    d->copyAppend(1, ch.unicode());
    d.data()[d.size] = '\0';
    return *this;
}

/*! \fn QString &QString::prepend(const QString &str)

    Prepends the string \a str to the beginning of this string and
    returns a reference to this string.

    This operation is typically very fast (\l{constant time}), because
    QString preallocates extra space at the beginning of the string data,
    so it can grow without reallocating the entire string each time.

    Example:

    \snippet qstring/main.cpp 36

    \sa append(), insert()
*/

/*! \fn QString &QString::prepend(QLatin1String str)

    \overload prepend()

    Prepends the Latin-1 string \a str to this string.
*/

/*! \fn QString &QString::prepend(const QChar *str, qsizetype len)
    \since 5.5
    \overload prepend()

    Prepends \a len characters from the QChar array \a str to this string and
    returns a reference to this string.
*/

/*! \fn QString &QString::prepend(QStringView str)
    \since 6.0
    \overload prepend()

    Prepends the string view \a str to the beginning of this string and
    returns a reference to this string.
*/

/*! \fn QString &QString::prepend(const QByteArray &ba)

    \overload prepend()

    Prepends the byte array \a ba to this string. The byte array is
    converted to Unicode using the fromUtf8() function.

    You can disable this function by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn QString &QString::prepend(const char *str)

    \overload prepend()

    Prepends the string \a str to this string. The const char pointer
    is converted to Unicode using the fromUtf8() function.

    You can disable this function by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn QString &QString::prepend(QChar ch)

    \overload prepend()

    Prepends the character \a ch to this string.
*/

/*!
  \fn QString &QString::remove(qsizetype position, qsizetype n)

  Removes \a n characters from the string, starting at the given \a
  position index, and returns a reference to the string.

  If the specified \a position index is within the string, but \a
  position + \a n is beyond the end of the string, the string is
  truncated at the specified \a position.

  \snippet qstring/main.cpp 37

//! [shrinking-erase]
  Element removal will preserve the string's capacity and not reduce the
  amount of allocated memory. To shed extra capacity and free as much memory
  as possible, call squeeze() after the last change to the string's size.
//! [shrinking-erase]

  \sa insert(), replace()
*/
QString &QString::remove(qsizetype pos, qsizetype len)
{
    if (pos < 0)  // count from end of string
        pos += size();
    if (size_t(pos) >= size_t(size())) {
        // range problems
    } else if (len >= size() - pos) {
        resize(pos); // truncate
    } else if (len > 0) {
        detach();
        d->erase(d.begin() + pos, len);
        d.data()[d.size] = u'\0';
    }
    return *this;
}

template<typename T>
static void removeStringImpl(QString &s, const T &needle, Qt::CaseSensitivity cs)
{
    const auto needleSize = needle.size();
    if (!needleSize)
        return;

    // avoid detach if nothing to do:
    qsizetype i = s.indexOf(needle, 0, cs);
    if (i < 0)
        return;

    const auto beg = s.begin(); // detaches
    auto dst = beg + i;
    auto src = beg + i + needleSize;
    const auto end = s.end();
    // loop invariant: [beg, dst[ is partial result
    //                 [src, end[ still to be checked for needles
    while (src < end) {
        const auto i = s.indexOf(needle, src - beg, cs);
        const auto hit = i == -1 ? end : beg + i;
        const auto skipped = hit - src;
        memmove(dst, src, skipped * sizeof(QChar));
        dst += skipped;
        src = hit + needleSize;
    }
    s.truncate(dst - beg);
}

/*!
  Removes every occurrence of the given \a str string in this
  string, and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is
  case sensitive; otherwise the search is case insensitive.

  This is the same as \c replace(str, "", cs).

  \include qstring.cpp shrinking-erase

  \sa replace()
*/
QString &QString::remove(const QString &str, Qt::CaseSensitivity cs)
{
    const auto s = str.d.data();
    if (QtPrivate::q_points_into_range(s, d.data(), d.data() + d.size))
        removeStringImpl(*this, QStringView{QVarLengthArray(s, s + str.size())}, cs);
    else
        removeStringImpl(*this, qToStringViewIgnoringNull(str), cs);
    return *this;
}

/*!
  \since 5.11
  \overload

  Removes every occurrence of the given \a str string in this
  string, and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is
  case sensitive; otherwise the search is case insensitive.

  This is the same as \c replace(str, "", cs).

  \include qstring.cpp shrinking-erase

  \sa replace()
*/
QString &QString::remove(QLatin1String str, Qt::CaseSensitivity cs)
{
    removeStringImpl(*this, str, cs);
    return *this;
}

/*!
  Removes every occurrence of the character \a ch in this string, and
  returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 38

  This is the same as \c replace(ch, "", cs).

  \include qstring.cpp shrinking-erase

  \sa replace()
*/
QString &QString::remove(QChar ch, Qt::CaseSensitivity cs)
{
    const qsizetype idx = indexOf(ch, 0, cs);
    if (idx != -1) {
        const auto first = begin(); // implicit detach()
        auto last = end();
        if (cs == Qt::CaseSensitive) {
            last = std::remove(first + idx, last, ch);
        } else {
            const QChar c = ch.toCaseFolded();
            auto caseInsensEqual = [c](QChar x) {
                return c == x.toCaseFolded();
            };
            last = std::remove_if(first + idx, last, caseInsensEqual);
        }
        resize(last - first);
    }
    return *this;
}

/*!
  \fn QString &QString::remove(const QRegularExpression &re)
  \since 5.0

  Removes every occurrence of the regular expression \a re in the
  string, and returns a reference to the string. For example:

  \snippet qstring/main.cpp 96

  \include qstring.cpp shrinking-erase

  \sa indexOf(), lastIndexOf(), replace()
*/

/*!
  \fn template <typename Predicate> QString &QString::removeIf(Predicate pred)
  \since 6.1

  Removes all elements for which the predicate \a pred returns true
  from the string. Returns a reference to the string.

  \sa remove()
*/

/*!
  \fn QString &QString::replace(qsizetype position, qsizetype n, const QString &after)

  Replaces \a n characters beginning at index \a position with
  the string \a after and returns a reference to this string.

  \note If the specified \a position index is within the string,
  but \a position + \a n goes outside the strings range,
  then \a n will be adjusted to stop at the end of the string.

  Example:

  \snippet qstring/main.cpp 40

  \sa insert(), remove()
*/
QString &QString::replace(qsizetype pos, qsizetype len, const QString &after)
{
    return replace(pos, len, after.constData(), after.length());
}

/*!
  \fn QString &QString::replace(qsizetype position, qsizetype n, const QChar *unicode, qsizetype size)
  \overload replace()
  Replaces \a n characters beginning at index \a position with the
  first \a size characters of the QChar array \a unicode and returns a
  reference to this string.
*/
QString &QString::replace(qsizetype pos, qsizetype len, const QChar *unicode, qsizetype size)
{
    if (size_t(pos) > size_t(this->size()))
        return *this;
    if (len > this->size() - pos)
        len = this->size() - pos;

    size_t index = pos;
    replace_helper(&index, 1, len, unicode, size);
    return *this;
}

/*!
  \fn QString &QString::replace(qsizetype position, qsizetype n, QChar after)
  \overload replace()

  Replaces \a n characters beginning at index \a position with the
  character \a after and returns a reference to this string.
*/
QString &QString::replace(qsizetype pos, qsizetype len, QChar after)
{
    return replace(pos, len, &after, 1);
}

/*!
  \overload replace()
  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 41

  \note The replacement text is not rescanned after it is inserted.

  Example:

  \snippet qstring/main.cpp 86
*/
QString &QString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    return replace(before.constData(), before.size(), after.constData(), after.size(), cs);
}

namespace { // helpers for replace and its helper:
QChar *textCopy(const QChar *start, qsizetype len)
{
    const size_t size = len * sizeof(QChar);
    QChar *const copy = static_cast<QChar *>(::malloc(size));
    Q_CHECK_PTR(copy);
    ::memcpy(copy, start, size);
    return copy;
}

static bool pointsIntoRange(const QChar *ptr, const char16_t *base, qsizetype len)
{
    const QChar *const start = reinterpret_cast<const QChar *>(base);
    const std::less<const QChar *> less;
    return !less(ptr, start) && less(ptr, start + len);
}
} // end namespace

/*!
  \internal
 */
void QString::replace_helper(size_t *indices, qsizetype nIndices, qsizetype blen, const QChar *after, qsizetype alen)
{
    // Copy after if it lies inside our own d.b area (which we could
    // possibly invalidate via a realloc or modify by replacement).
    QChar *afterBuffer = nullptr;
    if (pointsIntoRange(after, d.data(), d.size)) // Use copy in place of vulnerable original:
        after = afterBuffer = textCopy(after, alen);

    QT_TRY {
        if (blen == alen) {
            // replace in place
            detach();
            for (qsizetype i = 0; i < nIndices; ++i)
                memcpy(d.data() + indices[i], after, alen * sizeof(QChar));
        } else if (alen < blen) {
            // replace from front
            detach();
            size_t to = indices[0];
            if (alen)
                memcpy(d.data()+to, after, alen*sizeof(QChar));
            to += alen;
            size_t movestart = indices[0] + blen;
            for (qsizetype i = 1; i < nIndices; ++i) {
                qsizetype msize = indices[i] - movestart;
                if (msize > 0) {
                    memmove(d.data() + to, d.data() + movestart, msize * sizeof(QChar));
                    to += msize;
                }
                if (alen) {
                    memcpy(d.data() + to, after, alen * sizeof(QChar));
                    to += alen;
                }
                movestart = indices[i] + blen;
            }
            qsizetype msize = d.size - movestart;
            if (msize > 0)
                memmove(d.data() + to, d.data() + movestart, msize * sizeof(QChar));
            resize(d.size - nIndices*(blen-alen));
        } else {
            // replace from back
            qsizetype adjust = nIndices*(alen-blen);
            qsizetype newLen = d.size + adjust;
            qsizetype moveend = d.size;
            resize(newLen);

            while (nIndices) {
                --nIndices;
                qsizetype movestart = indices[nIndices] + blen;
                qsizetype insertstart = indices[nIndices] + nIndices*(alen-blen);
                qsizetype moveto = insertstart + alen;
                memmove(d.data() + moveto, d.data() + movestart,
                        (moveend - movestart)*sizeof(QChar));
                memcpy(d.data() + insertstart, after, alen * sizeof(QChar));
                moveend = movestart-blen;
            }
        }
    } QT_CATCH(const std::bad_alloc &) {
        ::free(afterBuffer);
        QT_RETHROW;
    }
    ::free(afterBuffer);
}

/*!
  \since 4.5
  \overload replace()

  Replaces each occurrence in this string of the first \a blen
  characters of \a before with the first \a alen characters of \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.
*/
QString &QString::replace(const QChar *before, qsizetype blen,
                          const QChar *after, qsizetype alen,
                          Qt::CaseSensitivity cs)
{
    if (d.size == 0) {
        if (blen)
            return *this;
    } else {
        if (cs == Qt::CaseSensitive && before == after && blen == alen)
            return *this;
    }
    if (alen == 0 && blen == 0)
        return *this;

    QStringMatcher matcher(before, blen, cs);
    QChar *beforeBuffer = nullptr, *afterBuffer = nullptr;

    qsizetype index = 0;
    while (1) {
        size_t indices[1024];
        size_t pos = 0;
        while (pos < 1024) {
            index = matcher.indexIn(*this, index);
            if (index == -1)
                break;
            indices[pos++] = index;
            if (blen) // Step over before:
                index += blen;
            else // Only count one instance of empty between any two characters:
                index++;
        }
        if (!pos) // Nothing to replace
            break;

        if (Q_UNLIKELY(index != -1)) {
            /*
              We're about to change data, that before and after might point
              into, and we'll need that data for our next batch of indices.
            */
            if (!afterBuffer && pointsIntoRange(after, d.data(), d.size))
                after = afterBuffer = textCopy(after, alen);

            if (!beforeBuffer && pointsIntoRange(before, d.data(), d.size)) {
                beforeBuffer = textCopy(before, blen);
                matcher = QStringMatcher(beforeBuffer, blen, cs);
            }
        }

        replace_helper(indices, pos, blen, after, alen);

        if (Q_LIKELY(index == -1)) // Nothing left to replace
            break;
        // The call to replace_helper just moved what index points at:
        index += pos*(alen-blen);
    }
    ::free(afterBuffer);
    ::free(beforeBuffer);

    return *this;
}

/*!
  \overload replace()
  Replaces every occurrence of the character \a ch in the string with
  \a after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.
*/
QString& QString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
    if (after.size() == 0)
        return remove(ch, cs);

    if (after.size() == 1)
        return replace(ch, after.front(), cs);

    if (size() == 0)
        return *this;

    char16_t cc = (cs == Qt::CaseSensitive ? ch.unicode() : ch.toCaseFolded().unicode());

    qsizetype index = 0;
    while (1) {
        size_t indices[1024];
        size_t pos = 0;
        if (cs == Qt::CaseSensitive) {
            while (pos < 1024 && index < size()) {
                if (d.data()[index] == cc)
                    indices[pos++] = index;
                index++;
            }
        } else {
            while (pos < 1024 && index < size()) {
                if (QChar::toCaseFolded(d.data()[index]) == cc)
                    indices[pos++] = index;
                index++;
            }
        }
        if (!pos) // Nothing to replace
            break;

        replace_helper(indices, pos, 1, after.constData(), after.size());

        if (Q_LIKELY(index == size())) // Nothing left to replace
            break;
        // The call to replace_helper just moved what index points at:
        index += pos*(after.size() - 1);
    }
    return *this;
}

/*!
  \overload replace()
  Replaces every occurrence of the character \a before with the
  character \a after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.
*/
QString& QString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
    if (d.size) {
        const qsizetype idx = indexOf(before, 0, cs);
        if (idx != -1) {
            detach();
            const char16_t a = after.unicode();
            char16_t *i = d.data();
            char16_t *const e = i + d.size;
            i += idx;
            *i = a;
            if (cs == Qt::CaseSensitive) {
                const char16_t b = before.unicode();
                while (++i != e) {
                    if (*i == b)
                        *i = a;
                }
            } else {
                const char16_t b = foldCase(before.unicode());
                while (++i != e) {
                    if (foldCase(*i) == b)
                        *i = a;
                }
            }
        }
    }
    return *this;
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(QLatin1String before, QLatin1String after, Qt::CaseSensitivity cs)
{
    qsizetype alen = after.size();
    qsizetype blen = before.size();
    QVarLengthArray<char16_t> a(alen);
    QVarLengthArray<char16_t> b(blen);
    qt_from_latin1(a.data(), after.latin1(), alen);
    qt_from_latin1(b.data(), before.latin1(), blen);
    return replace((const QChar *)b.data(), blen, (const QChar *)a.data(), alen, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(QLatin1String before, const QString &after, Qt::CaseSensitivity cs)
{
    qsizetype blen = before.size();
    QVarLengthArray<char16_t> b(blen);
    qt_from_latin1(b.data(), before.latin1(), blen);
    return replace((const QChar *)b.data(), blen, after.constData(), after.d.size, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the string \a before with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(const QString &before, QLatin1String after, Qt::CaseSensitivity cs)
{
    qsizetype alen = after.size();
    QVarLengthArray<char16_t> a(alen);
    qt_from_latin1(a.data(), after.latin1(), alen);
    return replace(before.constData(), before.d.size, (const QChar *)a.data(), alen, cs);
}

/*!
  \since 4.5
  \overload replace()

  Replaces every occurrence of the character \a c with the string \a
  after and returns a reference to this string.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note The text is not rescanned after a replacement.
*/
QString &QString::replace(QChar c, QLatin1String after, Qt::CaseSensitivity cs)
{
    qsizetype alen = after.size();
    QVarLengthArray<char16_t> a(alen);
    qt_from_latin1(a.data(), after.latin1(), alen);
    return replace(&c, 1, (const QChar *)a.data(), alen, cs);
}


/*!
    \fn bool QString::operator==(const QString &s1, const QString &s2)
    \overload operator==()

    Returns \c true if string \a s1 is equal to string \a s2; otherwise
    returns \c false.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator==(const QString &s1, QLatin1String s2)

    \overload operator==()

    Returns \c true if \a s1 is equal to \a s2; otherwise
    returns \c false.
*/

/*!
    \fn bool QString::operator==(QLatin1String s1, const QString &s2)

    \overload operator==()

    Returns \c true if \a s1 is equal to \a s2; otherwise
    returns \c false.
*/

/*! \fn bool QString::operator==(const QByteArray &other) const

    \overload operator==()

    The \a other byte array is converted to a QString using the
    fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    Returns \c true if this string is lexically equal to the parameter
    string \a other. Otherwise returns \c false.
*/

/*! \fn bool QString::operator==(const char *other) const

    \overload operator==()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \fn bool QString::operator<(const QString &s1, const QString &s2)

    \overload operator<()

    Returns \c true if string \a s1 is lexically less than string
    \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator<(const QString &s1, QLatin1String s2)

    \overload operator<()

    Returns \c true if \a s1 is lexically less than \a s2;
    otherwise returns \c false.
*/

/*!
    \fn bool QString::operator<(QLatin1String s1, const QString &s2)

    \overload operator<()

    Returns \c true if \a s1 is lexically less than \a s2;
    otherwise returns \c false.
*/

/*! \fn bool QString::operator<(const QByteArray &other) const

    \overload operator<()

    The \a other byte array is converted to a QString using the
    fromUtf8() function. If any NUL characters ('\\0') are embedded
    in the byte array, they will be included in the transformation.

    You can disable this operator
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QString::operator<(const char *other) const

    Returns \c true if this string is lexically less than string \a other.
    Otherwise returns \c false.

    \overload operator<()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QString::operator<=(const QString &s1, const QString &s2)

    Returns \c true if string \a s1 is lexically less than or equal to
    string \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator<=(const QString &s1, QLatin1String s2)

    \overload operator<=()

    Returns \c true if \a s1 is lexically less than or equal to \a s2;
    otherwise returns \c false.
*/

/*!
    \fn bool QString::operator<=(QLatin1String s1, const QString &s2)

    \overload operator<=()

    Returns \c true if \a s1 is lexically less than or equal to \a s2;
    otherwise returns \c false.
*/

/*! \fn bool QString::operator<=(const QByteArray &other) const

    \overload operator<=()

    The \a other byte array is converted to a QString using the
    fromUtf8() function. If any NUL characters ('\\0') are embedded
    in the byte array, they will be included in the transformation.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QString::operator<=(const char *other) const

    \overload operator<=()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QString::operator>(const QString &s1, const QString &s2)

    Returns \c true if string \a s1 is lexically greater than string \a s2;
    otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator>(const QString &s1, QLatin1String s2)

    \overload operator>()

    Returns \c true if \a s1 is lexically greater than \a s2;
    otherwise returns \c false.
*/

/*!
    \fn bool QString::operator>(QLatin1String s1, const QString &s2)

    \overload operator>()

    Returns \c true if \a s1 is lexically greater than \a s2;
    otherwise returns \c false.
*/

/*! \fn bool QString::operator>(const QByteArray &other) const

    \overload operator>()

    The \a other byte array is converted to a QString using the
    fromUtf8() function. If any NUL characters ('\\0') are embedded
    in the byte array, they will be included in the transformation.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QString::operator>(const char *other) const

    \overload operator>()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn bool QString::operator>=(const QString &s1, const QString &s2)

    Returns \c true if string \a s1 is lexically greater than or equal to
    string \a s2; otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator>=(const QString &s1, QLatin1String s2)

    \overload operator>=()

    Returns \c true if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns \c false.
*/

/*!
    \fn bool QString::operator>=(QLatin1String s1, const QString &s2)

    \overload operator>=()

    Returns \c true if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns \c false.
*/

/*! \fn bool QString::operator>=(const QByteArray &other) const

    \overload operator>=()

    The \a other byte array is converted to a QString using the
    fromUtf8() function. If any NUL characters ('\\0') are embedded in
    the byte array, they will be included in the transformation.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn bool QString::operator>=(const char *other) const

    \overload operator>=()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn bool QString::operator!=(const QString &s1, const QString &s2)

    Returns \c true if string \a s1 is not equal to string \a s2;
    otherwise returns \c false.

    \sa {Comparing Strings}
*/

/*! \fn bool QString::operator!=(const QString &s1, QLatin1String s2)

    Returns \c true if string \a s1 is not equal to string \a s2.
    Otherwise returns \c false.

    \overload operator!=()
*/

/*! \fn bool QString::operator!=(const QByteArray &other) const

    \overload operator!=()

    The \a other byte array is converted to a QString using the
    fromUtf8() function. If any NUL characters ('\\0') are embedded
    in the byte array, they will be included in the transformation.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn bool QString::operator!=(const char *other) const

    \overload operator!=()

    The \a other const char pointer is converted to a QString using
    the fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
  Returns the index position of the first occurrence of the string \a
  str in this string, searching forward from index position \a
  from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 24

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa lastIndexOf(), contains(), count()
*/
qsizetype QString::indexOf(const QString &str, qsizetype from, Qt::CaseSensitivity cs) const
{
    return QtPrivate::findString(QStringView(unicode(), length()), from, QStringView(str.unicode(), str.length()), cs);
}

/*!
    \fn qsizetype QString::indexOf(QStringView str, qsizetype from, Qt::CaseSensitivity cs) const
    \since 5.14
    \overload indexOf()

    Returns the index position of the first occurrence of the string view \a str
    in this string, searching forward from index position \a from.
    Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    If \a from is -1, the search starts at the last character; if it is
    -2, at the next to last character and so on.

    \sa QStringView::indexOf(), lastIndexOf(), contains(), count()
*/

/*!
  \since 4.5
  Returns the index position of the first occurrence of the string \a
  str in this string, searching forward from index position \a
  from. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 24

  If \a from is -1, the search starts at the last character; if it is
  -2, at the next to last character and so on.

  \sa lastIndexOf(), contains(), count()
*/

qsizetype QString::indexOf(QLatin1String str, qsizetype from, Qt::CaseSensitivity cs) const
{
    return QtPrivate::findString(QStringView(unicode(), size()), from, str, cs);
}

/*!
    \overload indexOf()

    Returns the index position of the first occurrence of the
    character \a ch in the string, searching forward from index
    position \a from. Returns -1 if \a ch could not be found.
*/
qsizetype QString::indexOf(QChar ch, qsizetype from, Qt::CaseSensitivity cs) const
{
    return qFindChar(QStringView(unicode(), length()), ch, from, cs);
}

/*!
  Returns the index position of the last occurrence of the string \a
  str in this string, searching backward from index position \a
  from. If \a from is -1, the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 29

  \note When searching for a 0-length \a str, the match at the end of
  the data is excluded from the search by a negative \a from, even
  though \c{-1} is normally thought of as searching from the end of the
  string: the match at the end is \e after the last character, so it is
  excluded. To include such a final empty match, either give a positive
  value for \a from or omit the \a from parameter entirely.

  \sa indexOf(), contains(), count()
*/
qsizetype QString::lastIndexOf(const QString &str, qsizetype from, Qt::CaseSensitivity cs) const
{
    return QtPrivate::lastIndexOf(QStringView(*this), from, str, cs);
}

/*!
  \fn qsizetype QString::lastIndexOf(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
  \since 6.2
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 29

  \sa indexOf(), contains(), count()
*/


/*!
  \since 4.5
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string, searching backward from index position \a
  from. If \a from is -1, the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 29

  \note When searching for a 0-length \a str, the match at the end of
  the data is excluded from the search by a negative \a from, even
  though \c{-1} is normally thought of as searching from the end of the
  string: the match at the end is \e after the last character, so it is
  excluded. To include such a final empty match, either give a positive
  value for \a from or omit the \a from parameter entirely.

  \sa indexOf(), contains(), count()
*/
qsizetype QString::lastIndexOf(QLatin1String str, qsizetype from, Qt::CaseSensitivity cs) const
{
    return QtPrivate::lastIndexOf(*this, from, str, cs);
}

/*!
  \fn qsizetype QString::lastIndexOf(QLatin1String str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
  \since 6.2
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string \a
  str in this string. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  Example:

  \snippet qstring/main.cpp 29

  \sa indexOf(), contains(), count()
*/

/*!
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the character
  \a ch, searching backward from position \a from.
*/
qsizetype QString::lastIndexOf(QChar ch, qsizetype from, Qt::CaseSensitivity cs) const
{
    return qLastIndexOf(QStringView(*this), ch, from, cs);
}

/*!
  \fn qsizetype QString::lastIndexOf(QStringView str, qsizetype from, Qt::CaseSensitivity cs) const
  \since 5.14
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string view \a
  str in this string, searching backward from index position \a
  from. If \a from is -1, the search starts at the last
  character; if \a from is -2, at the next to last character and so
  on. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \note When searching for a 0-length \a str, the match at the end of
  the data is excluded from the search by a negative \a from, even
  though \c{-1} is normally thought of as searching from the end of the
  string: the match at the end is \e after the last character, so it is
  excluded. To include such a final empty match, either give a positive
  value for \a from or omit the \a from parameter entirely.

  \sa indexOf(), contains(), count()
*/

/*!
  \fn qsizetype QString::lastIndexOf(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
  \since 6.2
  \overload lastIndexOf()

  Returns the index position of the last occurrence of the string view \a
  str in this string. Returns -1 if \a str is not found.

  If \a cs is Qt::CaseSensitive (default), the search is case
  sensitive; otherwise the search is case insensitive.

  \sa indexOf(), contains(), count()
*/

#if QT_CONFIG(regularexpression)
struct QStringCapture
{
    qsizetype pos;
    qsizetype len;
    int no;
};
Q_DECLARE_TYPEINFO(QStringCapture, Q_PRIMITIVE_TYPE);

/*!
  \overload replace()
  \since 5.0

  Replaces every occurrence of the regular expression \a re in the
  string with \a after. Returns a reference to the string. For
  example:

  \snippet qstring/main.cpp 87

  For regular expressions containing capturing groups,
  occurrences of \b{\\1}, \b{\\2}, ..., in \a after are replaced
  with the string captured by the corresponding capturing group.

  \snippet qstring/main.cpp 88

  \sa indexOf(), lastIndexOf(), remove(), QRegularExpression, QRegularExpressionMatch
*/
QString &QString::replace(const QRegularExpression &re, const QString &after)
{
    if (!re.isValid()) {
        qWarning("QString::replace: invalid QRegularExpression object");
        return *this;
    }

    const QString copy(*this);
    QRegularExpressionMatchIterator iterator = re.globalMatch(copy);
    if (!iterator.hasNext()) // no matches at all
        return *this;

    reallocData(d.size, QArrayData::KeepSize);

    qsizetype numCaptures = re.captureCount();

    // 1. build the backreferences list, holding where the backreferences
    // are in the replacement string
    QList<QStringCapture> backReferences;
    const qsizetype al = after.length();
    const QChar *ac = after.unicode();

    for (qsizetype i = 0; i < al - 1; i++) {
        if (ac[i] == QLatin1Char('\\')) {
            int no = ac[i + 1].digitValue();
            if (no > 0 && no <= numCaptures) {
                QStringCapture backReference;
                backReference.pos = i;
                backReference.len = 2;

                if (i < al - 2) {
                    int secondDigit = ac[i + 2].digitValue();
                    if (secondDigit != -1 && ((no * 10) + secondDigit) <= numCaptures) {
                        no = (no * 10) + secondDigit;
                        ++backReference.len;
                    }
                }

                backReference.no = no;
                backReferences.append(backReference);
            }
        }
    }

    // 2. iterate on the matches. For every match, copy in chunks
    // - the part before the match
    // - the after string, with the proper replacements for the backreferences

    qsizetype newLength = 0; // length of the new string, with all the replacements
    qsizetype lastEnd = 0;
    QList<QStringView> chunks;
    const QStringView copyView{ copy }, afterView{ after };
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        qsizetype len;
        // add the part before the match
        len = match.capturedStart() - lastEnd;
        if (len > 0) {
            chunks << copyView.mid(lastEnd, len);
            newLength += len;
        }

        lastEnd = 0;
        // add the after string, with replacements for the backreferences
        for (const QStringCapture &backReference : qAsConst(backReferences)) {
            // part of "after" before the backreference
            len = backReference.pos - lastEnd;
            if (len > 0) {
                chunks << afterView.mid(lastEnd, len);
                newLength += len;
            }

            // backreference itself
            len = match.capturedLength(backReference.no);
            if (len > 0) {
                chunks << copyView.mid(match.capturedStart(backReference.no), len);
                newLength += len;
            }

            lastEnd = backReference.pos + backReference.len;
        }

        // add the last part of the after string
        len = afterView.length() - lastEnd;
        if (len > 0) {
            chunks << afterView.mid(lastEnd, len);
            newLength += len;
        }

        lastEnd = match.capturedEnd();
    }

    // 3. trailing string after the last match
    if (copyView.length() > lastEnd) {
        chunks << copyView.mid(lastEnd);
        newLength += copyView.length() - lastEnd;
    }

    // 4. assemble the chunks together
    resize(newLength);
    qsizetype i = 0;
    QChar *uc = data();
    for (const QStringView &chunk : qAsConst(chunks)) {
        qsizetype len = chunk.length();
        memcpy(uc + i, chunk.constData(), len * sizeof(QChar));
        i += len;
    }

    return *this;
}
#endif // QT_CONFIG(regularexpression)

/*!
    Returns the number of (potentially overlapping) occurrences of
    the string \a str in this string.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa contains(), indexOf()
*/

qsizetype QString::count(const QString &str, Qt::CaseSensitivity cs) const
{
    return QtPrivate::count(QStringView(unicode(), size()), QStringView(str.unicode(), str.size()), cs);
}

/*!
    \overload count()

    Returns the number of occurrences of character \a ch in the string.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa contains(), indexOf()
*/

qsizetype QString::count(QChar ch, Qt::CaseSensitivity cs) const
{
    return QtPrivate::count(QStringView(unicode(), size()), ch, cs);
}

/*!
    \since 6.0
    \overload count()
    Returns the number of (potentially overlapping) occurrences of the
    string view \a str in this string.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa contains(), indexOf()
*/
qsizetype QString::count(QStringView str, Qt::CaseSensitivity cs) const
{
    return QtPrivate::count(*this, str, cs);
}

/*! \fn bool QString::contains(const QString &str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    Returns \c true if this string contains an occurrence of the string
    \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    Example:
    \snippet qstring/main.cpp 17

    \sa indexOf(), count()
*/

/*! \fn bool QString::contains(QLatin1String str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.3

    \overload contains()

    Returns \c true if this string contains an occurrence of the latin-1 string
    \a str; otherwise returns \c false.
*/

/*! \fn bool QString::contains(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \overload contains()

    Returns \c true if this string contains an occurrence of the
    character \a ch; otherwise returns \c false.
*/

/*! \fn bool QString::contains(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.14
    \overload contains()

    Returns \c true if this string contains an occurrence of the string view
    \a str; otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \sa indexOf(), count()
*/

#if QT_CONFIG(regularexpression)
/*!
    \since 5.5

    Returns the index position of the first match of the regular
    expression \a re in the string, searching forward from index
    position \a from. Returns -1 if \a re didn't match anywhere.

    If the match is successful and \a rmatch is not \nullptr, it also
    writes the results of the match into the QRegularExpressionMatch object
    pointed to by \a rmatch.

    Example:

    \snippet qstring/main.cpp 93
*/
qsizetype QString::indexOf(const QRegularExpression &re, qsizetype from, QRegularExpressionMatch *rmatch) const
{
    if (!re.isValid()) {
        qWarning("QString::indexOf: invalid QRegularExpression object");
        return -1;
    }

    QRegularExpressionMatch match = re.match(*this, from);
    if (match.hasMatch()) {
        const qsizetype ret = match.capturedStart();
        if (rmatch)
            *rmatch = std::move(match);
        return ret;
    }

    return -1;
}

/*!
    \since 5.5

    Returns the index position of the last match of the regular
    expression \a re in the string, which starts before the index
    position \a from. If \a from is -1, the search starts at the last
    character; if \a from is -2, at the next to last character and so
    on. Returns -1 if \a re didn't match anywhere.

    If the match is successful and \a rmatch is not \nullptr, it also
    writes the results of the match into the QRegularExpressionMatch object
    pointed to by \a rmatch.

    Example:

    \snippet qstring/main.cpp 94

    \note Due to how the regular expression matching algorithm works,
    this function will actually match repeatedly from the beginning of
    the string until the position \a from is reached.

    \note When searching for a regular expression \a re that may match
    0 characters, the match at the end of the data is excluded from the
    search by a negative \a from, even though \c{-1} is normally
    thought of as searching from the end of the string: the match at
    the end is \e after the last character, so it is excluded. To
    include such a final empty match, either give a positive value for
    \a from or omit the \a from parameter entirely.
*/
qsizetype QString::lastIndexOf(const QRegularExpression &re, qsizetype from, QRegularExpressionMatch *rmatch) const
{
    if (!re.isValid()) {
        qWarning("QString::lastIndexOf: invalid QRegularExpression object");
        return -1;
    }

    qsizetype endpos = (from < 0) ? (size() + from + 1) : (from + 1);
    QRegularExpressionMatchIterator iterator = re.globalMatch(*this);
    qsizetype lastIndex = -1;
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        qsizetype start = match.capturedStart();
        if (start < endpos) {
            lastIndex = start;
            if (rmatch)
                *rmatch = std::move(match);
        } else {
            break;
        }
    }

    return lastIndex;
}

/*!
    \fn qsizetype QString::lastIndexOf(const QRegularExpression &re, QRegularExpressionMatch *rmatch = nullptr) const
    \since 6.2
    \overload lastIndexOf()

    Returns the index position of the last match of the regular
    expression \a re in the string. Returns -1 if \a re didn't match anywhere.

    If the match is successful and \a rmatch is not \nullptr, it also
    writes the results of the match into the QRegularExpressionMatch object
    pointed to by \a rmatch.

    Example:

    \snippet qstring/main.cpp 94

    \note Due to how the regular expression matching algorithm works,
    this function will actually match repeatedly from the beginning of
    the string until the end of the string is reached.
*/

/*!
    \since 5.1

    Returns \c true if the regular expression \a re matches somewhere in this
    string; otherwise returns \c false.

    If the match is successful and \a rmatch is not \nullptr, it also
    writes the results of the match into the QRegularExpressionMatch object
    pointed to by \a rmatch.

    \sa QRegularExpression::match()
*/

bool QString::contains(const QRegularExpression &re, QRegularExpressionMatch *rmatch) const
{
    if (!re.isValid()) {
        qWarning("QString::contains: invalid QRegularExpression object");
        return false;
    }
    QRegularExpressionMatch m = re.match(*this);
    bool hasMatch = m.hasMatch();
    if (hasMatch && rmatch)
        *rmatch = std::move(m);
    return hasMatch;
}

/*!
    \overload count()
    \since 5.0

    Returns the number of times the regular expression \a re matches
    in the string.

    For historical reasons, this function counts overlapping matches,
    so in the example below, there are four instances of "ana" or
    "ama":

    \snippet qstring/main.cpp 95

    This behavior is different from simply iterating over the matches
    in the string using QRegularExpressionMatchIterator.

    \sa QRegularExpression::globalMatch()
*/
qsizetype QString::count(const QRegularExpression &re) const
{
    if (!re.isValid()) {
        qWarning("QString::count: invalid QRegularExpression object");
        return 0;
    }
    qsizetype count = 0;
    qsizetype index = -1;
    qsizetype len = length();
    while (index <= len - 1) {
        QRegularExpressionMatch match = re.match(*this, index + 1);
        if (!match.hasMatch())
            break;
        index = match.capturedStart();
        count++;
    }
    return count;
}
#endif // QT_CONFIG(regularexpression)

/*! \fn qsizetype QString::count() const

    \overload count()

    Same as size().
*/


/*!
    \enum QString::SectionFlag

    This enum specifies flags that can be used to affect various
    aspects of the section() function's behavior with respect to
    separators and empty fields.

    \value SectionDefault Empty fields are counted, leading and
    trailing separators are not included, and the separator is
    compared case sensitively.

    \value SectionSkipEmpty Treat empty fields as if they don't exist,
    i.e. they are not considered as far as \e start and \e end are
    concerned.

    \value SectionIncludeLeadingSep Include the leading separator (if
    any) in the result string.

    \value SectionIncludeTrailingSep Include the trailing separator
    (if any) in the result string.

    \value SectionCaseInsensitiveSeps Compare the separator
    case-insensitively.

    \sa section()
*/

/*!
    \fn QString QString::section(QChar sep, qsizetype start, qsizetype end = -1, SectionFlags flags) const

    This function returns a section of the string.

    This string is treated as a sequence of fields separated by the
    character, \a sep. The returned string consists of the fields from
    position \a start to position \a end inclusive. If \a end is not
    specified, all fields from position \a start to the end of the
    string are included. Fields are numbered 0, 1, 2, etc., counting
    from the left, and -1, -2, etc., counting from right to left.

    The \a flags argument can be used to affect some aspects of the
    function's behavior, e.g. whether to be case sensitive, whether
    to skip empty fields and how to deal with leading and trailing
    separators; see \l{SectionFlags}.

    \snippet qstring/main.cpp 52

    If \a start or \a end is negative, we count fields from the right
    of the string, the right-most field being -1, the one from
    right-most field being -2, and so on.

    \snippet qstring/main.cpp 53

    \sa split()
*/

/*!
    \overload section()

    \snippet qstring/main.cpp 51
    \snippet qstring/main.cpp 54

    \sa split()
*/

QString QString::section(const QString &sep, qsizetype start, qsizetype end, SectionFlags flags) const
{
    const QList<QStringView> sections = QStringView{ *this }.split(
            sep, Qt::KeepEmptyParts, (flags & SectionCaseInsensitiveSeps) ? Qt::CaseInsensitive : Qt::CaseSensitive);
    const qsizetype sectionsSize = sections.size();
    if (!(flags & SectionSkipEmpty)) {
        if (start < 0)
            start += sectionsSize;
        if (end < 0)
            end += sectionsSize;
    } else {
        qsizetype skip = 0;
        for (qsizetype k = 0; k < sectionsSize; ++k) {
            if (sections.at(k).isEmpty())
                skip++;
        }
        if (start < 0)
            start += sectionsSize - skip;
        if (end < 0)
            end += sectionsSize - skip;
    }
    if (start >= sectionsSize || end < 0 || start > end)
        return QString();

    QString ret;
    qsizetype first_i = start, last_i = end;
    for (qsizetype x = 0, i = 0; x <= end && i < sectionsSize; ++i) {
        const QStringView &section = sections.at(i);
        const bool empty = section.isEmpty();
        if (x >= start) {
            if (x == start)
                first_i = i;
            if (x == end)
                last_i = i;
            if (x > start && i > 0)
                ret += sep;
            ret += section;
        }
        if (!empty || !(flags & SectionSkipEmpty))
            x++;
    }
    if ((flags & SectionIncludeLeadingSep) && first_i > 0)
        ret.prepend(sep);
    if ((flags & SectionIncludeTrailingSep) && last_i < sectionsSize - 1)
        ret += sep;
    return ret;
}

#if QT_CONFIG(regularexpression)
class qt_section_chunk {
public:
    qt_section_chunk() {}
    qt_section_chunk(qsizetype l, QStringView s) : length(l), string(std::move(s)) {}
    qsizetype length;
    QStringView string;
};
Q_DECLARE_TYPEINFO(qt_section_chunk, Q_RELOCATABLE_TYPE);

static QString extractSections(const QList<qt_section_chunk> &sections, qsizetype start, qsizetype end,
                               QString::SectionFlags flags)
{
    const qsizetype sectionsSize = sections.size();

    if (!(flags & QString::SectionSkipEmpty)) {
        if (start < 0)
            start += sectionsSize;
        if (end < 0)
            end += sectionsSize;
    } else {
        qsizetype skip = 0;
        for (qsizetype k = 0; k < sectionsSize; ++k) {
            const qt_section_chunk &section = sections.at(k);
            if (section.length == section.string.length())
                skip++;
        }
        if (start < 0)
            start += sectionsSize - skip;
        if (end < 0)
            end += sectionsSize - skip;
    }
    if (start >= sectionsSize || end < 0 || start > end)
        return QString();

    QString ret;
    qsizetype x = 0;
    qsizetype first_i = start, last_i = end;
    for (qsizetype i = 0; x <= end && i < sectionsSize; ++i) {
        const qt_section_chunk &section = sections.at(i);
        const bool empty = (section.length == section.string.length());
        if (x >= start) {
            if (x == start)
                first_i = i;
            if (x == end)
                last_i = i;
            if (x != start)
                ret += section.string;
            else
                ret += section.string.mid(section.length);
        }
        if (!empty || !(flags & QString::SectionSkipEmpty))
            x++;
    }

    if ((flags & QString::SectionIncludeLeadingSep) && first_i >= 0) {
        const qt_section_chunk &section = sections.at(first_i);
        ret.prepend(section.string.left(section.length));
    }

    if ((flags & QString::SectionIncludeTrailingSep)
        && last_i < sectionsSize - 1) {
        const qt_section_chunk &section = sections.at(last_i+1);
        ret += section.string.left(section.length);
    }

    return ret;
}

/*!
    \overload section()
    \since 5.0

    This string is treated as a sequence of fields separated by the
    regular expression, \a re.

    \snippet qstring/main.cpp 89

    \warning Using this QRegularExpression version is much more expensive than
    the overloaded string and character versions.

    \sa split(), simplified()
*/
QString QString::section(const QRegularExpression &re, qsizetype start, qsizetype end, SectionFlags flags) const
{
    if (!re.isValid()) {
        qWarning("QString::section: invalid QRegularExpression object");
        return QString();
    }

    const QChar *uc = unicode();
    if (!uc)
        return QString();

    QRegularExpression sep(re);
    if (flags & SectionCaseInsensitiveSeps)
        sep.setPatternOptions(sep.patternOptions() | QRegularExpression::CaseInsensitiveOption);

    QList<qt_section_chunk> sections;
    qsizetype n = length(), m = 0, last_m = 0, last_len = 0;
    QRegularExpressionMatchIterator iterator = sep.globalMatch(*this);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        m = match.capturedStart();
        sections.append(qt_section_chunk(last_len, QStringView{ *this }.mid(last_m, m - last_m)));
        last_m = m;
        last_len = match.capturedLength();
    }
    sections.append(qt_section_chunk(last_len, QStringView{ *this }.mid(last_m, n - last_m)));

    return extractSections(sections, start, end, flags);
}
#endif // QT_CONFIG(regularexpression)

/*!
    Returns a substring that contains the \a n leftmost characters
    of the string.

    If you know that \a n cannot be out of bounds, use first() instead in new
    code, because it is faster.

    The entire string is returned if \a n is greater than or equal
    to size(), or less than zero.

    \sa first(), last(), startsWith(), chopped(), chop(), truncate()
*/
QString QString::left(qsizetype n)  const
{
    if (size_t(n) >= size_t(size()))
        return *this;
    return QString((const QChar*) d.data(), n);
}

/*!
    Returns a substring that contains the \a n rightmost characters
    of the string.

    If you know that \a n cannot be out of bounds, use last() instead in new
    code, because it is faster.

    The entire string is returned if \a n is greater than or equal
    to size(), or less than zero.

    \sa endsWith(), last(), first(), sliced(), chopped(), chop(), truncate()
*/
QString QString::right(qsizetype n) const
{
    if (size_t(n) >= size_t(size()))
        return *this;
    return QString(constData() + size() - n, n);
}

/*!
    Returns a string that contains \a n characters of this string,
    starting at the specified \a position index.

    If you know that \a position and \a n cannot be out of bounds, use sliced()
    instead in new code, because it is faster.

    Returns a null string if the \a position index exceeds the
    length of the string. If there are less than \a n characters
    available in the string starting at the given \a position, or if
    \a n is -1 (default), the function returns all characters that
    are available from the specified \a position.


    \sa first(), last(), sliced(), chopped(), chop(), truncate()
*/

QString QString::mid(qsizetype position, qsizetype n) const
{
    qsizetype p = position;
    qsizetype l = n;
    using namespace QtPrivate;
    switch (QContainerImplHelper::mid(size(), &p, &l)) {
    case QContainerImplHelper::Null:
        return QString();
    case QContainerImplHelper::Empty:
        return QString(DataPointer::fromRawData(&_empty, 0));
    case QContainerImplHelper::Full:
        return *this;
    case QContainerImplHelper::Subset:
        return QString(constData() + p, l);
    }
    Q_UNREACHABLE();
    return QString();
}

/*!
    \fn QString QString::first(qsizetype n) const
    \since 6.0

    Returns a string that contains the first \a n characters
    of this string.

    \note The behavior is undefined when \a n < 0 or \a n > size().

    \snippet qstring/main.cpp 31

    \sa last(), sliced(), startsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QString QString::last(qsizetype n) const
    \since 6.0

    Returns the string that contains the last \a n characters of this string.

    \note The behavior is undefined when \a n < 0 or \a n > size().

    \snippet qstring/main.cpp 48

    \sa first(), sliced(), endsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QString QString::sliced(qsizetype pos, qsizetype n) const
    \since 6.0

    Returns a string that contains \a n characters of this string,
    starting at position \a pos.

    \note The behavior is undefined when \a pos < 0, \a n < 0,
    or \a pos + \a n > size().

    \snippet qstring/main.cpp 34

    \sa first(), last(), chopped(), chop(), truncate()
*/

/*!
    \fn QString QString::sliced(qsizetype pos) const
    \since 6.0
    \overload

    Returns a string that contains the portion of this string starting at
    position \a pos and extending to its end.

    \note The behavior is undefined when \a pos < 0 or \a pos > size().

    \sa first(), last(), sliced(), chopped(), chop(), truncate()
*/

/*!
    \fn QString QString::chopped(qsizetype len) const
    \since 5.10

    Returns a string that contains the size() - \a len leftmost characters
    of this string.

    \note The behavior is undefined if \a len is negative or greater than size().

    \sa endsWith(), first(), last(), sliced(), chop(), truncate()
*/

/*!
    Returns \c true if the string starts with \a s; otherwise returns
    \c false.

    If \a cs is Qt::CaseSensitive (default), the search is
    case sensitive; otherwise the search is case insensitive.

    \snippet qstring/main.cpp 65

    \sa endsWith()
*/
bool QString::startsWith(const QString& s, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, s, cs);
}

/*!
  \overload startsWith()
 */
bool QString::startsWith(QLatin1String s, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, s, cs);
}

/*!
  \overload startsWith()

  Returns \c true if the string starts with \a c; otherwise returns
  \c false.
*/
bool QString::startsWith(QChar c, Qt::CaseSensitivity cs) const
{
    return qt_starts_with(*this, c, cs);
}

/*!
    \fn bool QString::startsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \overload

    Returns \c true if the string starts with the string view \a str;
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case-sensitive;
    otherwise the search is case insensitive.

    \sa endsWith()
*/

/*!
    Returns \c true if the string ends with \a s; otherwise returns
    \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \snippet qstring/main.cpp 20

    \sa startsWith()
*/
bool QString::endsWith(const QString &s, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, s, cs);
}

/*!
    \fn bool QString::endsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \overload endsWith()
    Returns \c true if the string ends with the string view \a str;
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \sa startsWith()
*/

/*!
    \overload endsWith()
*/
bool QString::endsWith(QLatin1String s, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, s, cs);
}

/*!
  Returns \c true if the string ends with \a c; otherwise returns
  \c false.

  \overload endsWith()
 */
bool QString::endsWith(QChar c, Qt::CaseSensitivity cs) const
{
    return qt_ends_with(*this, c, cs);
}

/*!
    Returns \c true if the string is uppercase, that is, it's identical
    to its toUpper() folding.

    Note that this does \e not mean that the string does not contain
    lowercase letters (some lowercase letters do not have a uppercase
    folding; they are left unchanged by toUpper()).
    For more information, refer to the Unicode standard, section 3.13.

    \since 5.12

    \sa QChar::toUpper(), isLower()
*/
bool QString::isUpper() const
{
    QStringIterator it(*this);

    while (it.hasNext()) {
        const char32_t uc = it.next();
        if (qGetProp(uc)->cases[QUnicodeTables::UpperCase].diff)
            return false;
    }

    return true;
}

/*!
    Returns \c true if the string is lowercase, that is, it's identical
    to its toLower() folding.

    Note that this does \e not mean that the string does not contain
    uppercase letters (some uppercase letters do not have a lowercase
    folding; they are left unchanged by toLower()).
    For more information, refer to the Unicode standard, section 3.13.

    \since 5.12

    \sa QChar::toLower(), isUpper()
 */
bool QString::isLower() const
{
    QStringIterator it(*this);

    while (it.hasNext()) {
        const char32_t uc = it.next();
        if (qGetProp(uc)->cases[QUnicodeTables::LowerCase].diff)
            return false;
    }

    return true;
}

static QByteArray qt_convert_to_latin1(QStringView string);

QByteArray QString::toLatin1_helper(const QString &string)
{
    return qt_convert_to_latin1(string);
}

/*!
    \since 6.0
    \internal
    \relates QAnyStringView

    Returns a UTF-16 representation of \a string as a QString.

    \sa QString::toLatin1(), QStringView::toLatin1(), QtPrivate::convertToUtf8(),
    QtPrivate::convertToLocal8Bit(), QtPrivate::convertToUcs4()
*/
QString QtPrivate::convertToQString(QAnyStringView string)
{
    return string.visit([] (auto string) { return string.toString(); });
}

/*!
    \since 5.10
    \internal
    \relates QStringView

    Returns a Latin-1 representation of \a string as a QByteArray.

    The behavior is undefined if \a string contains non-Latin1 characters.

    \sa QString::toLatin1(), QStringView::toLatin1(), QtPrivate::convertToUtf8(),
    QtPrivate::convertToLocal8Bit(), QtPrivate::convertToUcs4()
*/
QByteArray QtPrivate::convertToLatin1(QStringView string)
{
    return qt_convert_to_latin1(string);
}

static QByteArray qt_convert_to_latin1(QStringView string)
{
    if (Q_UNLIKELY(string.isNull()))
        return QByteArray();

    QByteArray ba(string.length(), Qt::Uninitialized);

    // since we own the only copy, we're going to const_cast the constData;
    // that avoids an unnecessary call to detach() and expansion code that will never get used
    qt_to_latin1(reinterpret_cast<uchar *>(const_cast<char *>(ba.constData())),
                 string.utf16(), string.size());
    return ba;
}

QByteArray QString::toLatin1_helper_inplace(QString &s)
{
    if (!s.isDetached())
        return qt_convert_to_latin1(s);

    // We can return our own buffer to the caller.
    // Conversion to Latin-1 always shrinks the buffer by half.
    const char16_t *data = s.d.data();
    qsizetype length = s.d.size;

    // Move the d pointer over to the bytearray.
    // Kids, avert your eyes. Don't try this at home.

    // this relies on the fact that we use QArrayData for everything behind the scenes which has the same layout
    static_assert(sizeof(QByteArray::DataPointer) == sizeof(QString::DataPointer), "sizes have to be equal");
    QByteArray::DataPointer ba_d(reinterpret_cast<QByteArray::Data *>(s.d.d_ptr()), reinterpret_cast<char *>(s.d.data()), length);
    ba_d.ref();
    s.clear();

    char *ddata = ba_d.data();

    // multiply the allocated capacity by sizeof(char16_t)
    ba_d.d_ptr()->alloc *= sizeof(char16_t);

    // do the in-place conversion
    qt_to_latin1(reinterpret_cast<uchar *>(ddata), data, length);
    ddata[length] = '\0';
    return QByteArray(ba_d);
}

/*!
    \fn QByteArray QString::toLatin1() const

    Returns a Latin-1 representation of the string as a QByteArray.

    The returned byte array is undefined if the string contains non-Latin1
    characters. Those characters may be suppressed or replaced with a
    question mark.

    \sa fromLatin1(), toUtf8(), toLocal8Bit(), QStringEncoder
*/

static QByteArray qt_convert_to_local_8bit(QStringView string);

/*!
    \fn QByteArray QString::toLocal8Bit() const

    Returns the local 8-bit representation of the string as a
    QByteArray. The returned byte array is undefined if the string
    contains characters not supported by the local 8-bit encoding.

    On Unix systems this is equivalen to toUtf8(), on Windows the systems
    current code page is being used.

    If this string contains any characters that cannot be encoded in the
    locale, the returned byte array is undefined. Those characters may be
    suppressed or replaced by another.

    \sa fromLocal8Bit(), toLatin1(), toUtf8(), QStringEncoder
*/

QByteArray QString::toLocal8Bit_helper(const QChar *data, qsizetype size)
{
    return qt_convert_to_local_8bit(QStringView(data, size));
}

static QByteArray qt_convert_to_local_8bit(QStringView string)
{
    if (string.isNull())
        return QByteArray();
    QStringEncoder fromUtf16(QStringEncoder::System, QStringEncoder::Flag::Stateless);
    return fromUtf16(string);
}

/*!
    \since 5.10
    \internal
    \relates QStringView

    Returns a local 8-bit representation of \a string as a QByteArray.

    On Unix systems this is equivalen to toUtf8(), on Windows the systems
    current code page is being used.

    The behavior is undefined if \a string contains characters not
    supported by the locale's 8-bit encoding.

    \sa QString::toLocal8Bit(), QStringView::toLocal8Bit()
*/
QByteArray QtPrivate::convertToLocal8Bit(QStringView string)
{
    return qt_convert_to_local_8bit(string);
}

static QByteArray qt_convert_to_utf8(QStringView str);

/*!
    \fn QByteArray QString::toUtf8() const

    Returns a UTF-8 representation of the string as a QByteArray.

    UTF-8 is a Unicode codec and can represent all characters in a Unicode
    string like QString.

    \sa fromUtf8(), toLatin1(), toLocal8Bit(), QStringEncoder
*/

QByteArray QString::toUtf8_helper(const QString &str)
{
    return qt_convert_to_utf8(str);
}

static QByteArray qt_convert_to_utf8(QStringView str)
{
    if (str.isNull())
        return QByteArray();

    return QUtf8::convertFromUnicode(str);
}

/*!
    \since 5.10
    \internal
    \relates QStringView

    Returns a UTF-8 representation of \a string as a QByteArray.

    UTF-8 is a Unicode codec and can represent all characters in a Unicode
    string like QStringView.

    \sa QString::toUtf8(), QStringView::toUtf8()
*/
QByteArray QtPrivate::convertToUtf8(QStringView string)
{
    return qt_convert_to_utf8(string);
}

static QList<uint> qt_convert_to_ucs4(QStringView string);

/*!
    \since 4.2

    Returns a UCS-4/UTF-32 representation of the string as a QList<uint>.

    UCS-4 is a Unicode codec and therefore it is lossless. All characters from
    this string will be encoded in UCS-4. Any invalid sequence of code units in
    this string is replaced by the Unicode's replacement character
    (QChar::ReplacementCharacter, which corresponds to \c{U+FFFD}).

    The returned list is not \\0'-terminated.

    \sa fromUtf8(), toUtf8(), toLatin1(), toLocal8Bit(), QStringEncoder,
        fromUcs4(), toWCharArray()
*/
QList<uint> QString::toUcs4() const
{
    return qt_convert_to_ucs4(*this);
}

static QList<uint> qt_convert_to_ucs4(QStringView string)
{
    QList<uint> v(string.length());
    uint *a = const_cast<uint*>(v.constData());
    QStringIterator it(string);
    while (it.hasNext())
        *a++ = it.next();
    v.resize(a - v.constData());
    return v;
}

/*!
    \since 5.10
    \internal
    \relates QStringView

    Returns a UCS-4/UTF-32 representation of \a string as a QList<uint>.

    UCS-4 is a Unicode codec and therefore it is lossless. All characters from
    this string will be encoded in UCS-4. Any invalid sequence of code units in
    this string is replaced by the Unicode's replacement character
    (QChar::ReplacementCharacter, which corresponds to \c{U+FFFD}).

    The returned list is not \\0'-terminated.

    \sa QString::toUcs4(), QStringView::toUcs4(), QtPrivate::convertToLatin1(),
    QtPrivate::convertToLocal8Bit(), QtPrivate::convertToUtf8()
*/
QList<uint> QtPrivate::convertToUcs4(QStringView string)
{
    return qt_convert_to_ucs4(string);
}

/*!
    \fn QString QString::fromLatin1(QByteArrayView str)
    \overload
    \since 6.0

    Returns a QString initialized with the Latin-1 string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000).
*/
QString QString::fromLatin1(QByteArrayView ba)
{
    DataPointer d;
    if (!ba.data()) {
        // nothing to do
    } else if (ba.size() == 0) {
        d = DataPointer::fromRawData(&_empty, 0);
    } else {
        d = DataPointer(Data::allocate(ba.size()), ba.size());
        Q_CHECK_PTR(d.data());
        d.data()[ba.size()] = '\0';
        char16_t *dst = d.data();

        qt_from_latin1(dst, ba.data(), size_t(ba.size()));
    }
    return QString(std::move(d));
}

/*!
    \fn QString QString::fromLatin1(const char *str, qsizetype size)
    Returns a QString initialized with the first \a size characters
    of the Latin-1 string \a str.

    If \a size is \c{-1}, \c{strlen(str)} is used instead.

    \sa toLatin1(), fromUtf8(), fromLocal8Bit()
*/

/*!
    \fn QString QString::fromLatin1(const QByteArray &str)
    \overload
    \since 5.0

    Returns a QString initialized with the Latin-1 string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000). This behavior is
    different from Qt 5.x.
*/

/*!
    \fn QString QString::fromLocal8Bit(const char *str, qsizetype size)
    Returns a QString initialized with the first \a size characters
    of the 8-bit string \a str.

    If \a size is \c{-1}, \c{strlen(str)} is used instead.

    On Unix systems this is equivalen to fromUtf8(), on Windows the systems
    current code page is being used.

    \sa toLocal8Bit(), fromLatin1(), fromUtf8()
*/

/*!
    \fn QString QString::fromLocal8Bit(const QByteArray &str)
    \overload
    \since 5.0

    Returns a QString initialized with the 8-bit string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000). This behavior is
    different from Qt 5.x.
*/

/*!
    \fn QString QString::fromLocal8Bit(QByteArrayView str)
    \overload
    \since 6.0

    Returns a QString initialized with the 8-bit string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000).
*/
QString QString::fromLocal8Bit(QByteArrayView ba)
{
    if (ba.isNull())
        return QString();
    if (ba.isEmpty())
        return QString(DataPointer::fromRawData(&_empty, 0));
    QStringDecoder toUtf16(QStringDecoder::System, QStringDecoder::Flag::Stateless);
    return toUtf16(ba);
}

/*! \fn QString QString::fromUtf8(const char *str, qsizetype size)
    Returns a QString initialized with the first \a size bytes
    of the UTF-8 string \a str.

    If \a size is \c{-1}, \c{strlen(str)} is used instead.

    UTF-8 is a Unicode codec and can represent all characters in a Unicode
    string like QString. However, invalid sequences are possible with UTF-8
    and, if any such are found, they will be replaced with one or more
    "replacement characters", or suppressed. These include non-Unicode
    sequences, non-characters, overlong sequences or surrogate codepoints
    encoded into UTF-8.

    This function can be used to process incoming data incrementally as long as
    all UTF-8 characters are terminated within the incoming data. Any
    unterminated characters at the end of the string will be replaced or
    suppressed. In order to do stateful decoding, please use \l QStringDecoder.

    \sa toUtf8(), fromLatin1(), fromLocal8Bit()
*/

/*!
    \fn QString QString::fromUtf8(const char8_t *str)
    \overload
    \since 6.1

    This overload is only available when compiling in C++20 mode.
*/

/*!
    \fn QString QString::fromUtf8(const char8_t *str, qsizetype size)
    \overload
    \since 6.0

    This overload is only available when compiling in C++20 mode.
*/

/*!
    \fn QString QString::fromUtf8(const QByteArray &str)
    \overload
    \since 5.0

    Returns a QString initialized with the UTF-8 string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000). This behavior is
    different from Qt 5.x.
*/

/*!
    \fn QString QString::fromUtf8(QByteArrayView str)
    \overload
    \since 6.0

    Returns a QString initialized with the UTF-8 string \a str.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, converted to Unicode null characters (U+0000).
*/
QString QString::fromUtf8(QByteArrayView ba)
{
    if (ba.isNull())
        return QString();
    if (ba.isEmpty())
        return QString(DataPointer::fromRawData(&_empty, 0));
    return QUtf8::convertToUnicode(ba);
}

/*!
    \since 5.3
    Returns a QString initialized with the first \a size characters
    of the Unicode string \a unicode (ISO-10646-UTF-16 encoded).

    If \a size is -1 (default), \a unicode must be \\0'-terminated.

    This function checks for a Byte Order Mark (BOM). If it is missing,
    host byte order is assumed.

    This function is slow compared to the other Unicode conversions.
    Use QString(const QChar *, qsizetype) or QString(const QChar *) if possible.

    QString makes a deep copy of the Unicode data.

    \sa utf16(), setUtf16(), fromStdU16String()
*/
QString QString::fromUtf16(const char16_t *unicode, qsizetype size)
{
    if (!unicode)
        return QString();
    if (size < 0)
        size = QtPrivate::qustrlen(unicode);
    QStringDecoder toUtf16(QStringDecoder::Utf16, QStringDecoder::Flag::Stateless);
    return toUtf16(QByteArrayView(reinterpret_cast<const char *>(unicode), size * 2));
}

/*!
    \fn QString QString::fromUtf16(const ushort *str, qsizetype size)
    \deprecated [6.0] Use the \c char16_t overload instead.
*/

/*!
    \fn QString QString::fromUcs4(const uint *str, qsizetype size)
    \since 4.2
    \deprecated [6.0] Use the \c char32_t overload instead.
*/

/*!
    \since 5.3

    Returns a QString initialized with the first \a size characters
    of the Unicode string \a unicode (ISO-10646-UCS-4 encoded).

    If \a size is -1 (default), \a unicode must be \\0'-terminated.

    \sa toUcs4(), fromUtf16(), utf16(), setUtf16(), fromWCharArray(),
        fromStdU32String()
*/
QString QString::fromUcs4(const char32_t *unicode, qsizetype size)
{
    if (!unicode)
        return QString();
    if (size < 0) {
        size = 0;
        while (unicode[size] != 0)
            ++size;
    }
    QStringDecoder toUtf16(QStringDecoder::Utf32, QStringDecoder::Flag::Stateless);
    return toUtf16(QByteArrayView(reinterpret_cast<const char *>(unicode), size * 4));
}


/*!
    Resizes the string to \a size characters and copies \a unicode
    into the string.

    If \a unicode is \nullptr, nothing is copied, but the string is still
    resized to \a size.

    \sa unicode(), setUtf16()
*/
QString& QString::setUnicode(const QChar *unicode, qsizetype size)
{
     resize(size);
     if (unicode && size)
         memcpy(d.data(), unicode, size * sizeof(QChar));
     return *this;
}

/*!
    \fn QString &QString::setUtf16(const ushort *unicode, qsizetype size)

    Resizes the string to \a size characters and copies \a unicode
    into the string.

    If \a unicode is \nullptr, nothing is copied, but the string is still
    resized to \a size.

    Note that unlike fromUtf16(), this function does not consider BOMs and
    possibly differing byte ordering.

    \sa utf16(), setUnicode()
*/

/*!
    \fn QString QString::simplified() const

    Returns a string that has whitespace removed from the start
    and the end, and that has each sequence of internal whitespace
    replaced with a single space.

    Whitespace means any character for which QChar::isSpace() returns
    \c true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.

    Example:

    \snippet qstring/main.cpp 57

    \sa trimmed()
*/
QString QString::simplified_helper(const QString &str)
{
    return QStringAlgorithms<const QString>::simplified_helper(str);
}

QString QString::simplified_helper(QString &str)
{
    return QStringAlgorithms<QString>::simplified_helper(str);
}

namespace {
    template <typename StringView>
    StringView qt_trimmed(StringView s) noexcept
    {
        auto begin = s.begin();
        auto end = s.end();
        QStringAlgorithms<const StringView>::trimmed_helper_positions(begin, end);
        return StringView{begin, end};
    }
}

/*!
    \fn QStringView QtPrivate::trimmed(QStringView s)
    \fn QLatin1String QtPrivate::trimmed(QLatin1String s)
    \internal
    \relates QStringView
    \since 5.10

    Returns \a s with whitespace removed from the start and the end.

    Whitespace means any character for which QChar::isSpace() returns
    \c true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.

    \sa QString::trimmed(), QStringView::trimmed(), QLatin1String::trimmed()
*/
QStringView QtPrivate::trimmed(QStringView s) noexcept
{
    return qt_trimmed(s);
}

QLatin1String QtPrivate::trimmed(QLatin1String s) noexcept
{
    return qt_trimmed(s);
}

/*!
    \fn QString QString::trimmed() const

    Returns a string that has whitespace removed from the start and
    the end.

    Whitespace means any character for which QChar::isSpace() returns
    \c true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.

    Example:

    \snippet qstring/main.cpp 82

    Unlike simplified(), trimmed() leaves internal whitespace alone.

    \sa simplified()
*/
QString QString::trimmed_helper(const QString &str)
{
    return QStringAlgorithms<const QString>::trimmed_helper(str);
}

QString QString::trimmed_helper(QString &str)
{
    return QStringAlgorithms<QString>::trimmed_helper(str);
}

/*! \fn const QChar QString::at(qsizetype position) const

    Returns the character at the given index \a position in the
    string.

    The \a position must be a valid index position in the string
    (i.e., 0 <= \a position < size()).

    \sa operator[]()
*/

/*!
    \fn QChar &QString::operator[](qsizetype position)

    Returns the character at the specified \a position in the string as a
    modifiable reference.

    Example:

    \snippet qstring/main.cpp 85

    \sa at()
*/

/*!
    \fn const QChar QString::operator[](qsizetype position) const

    \overload operator[]()
*/

/*!
    \fn QChar QString::front() const
    \since 5.10

    Returns the first character in the string.
    Same as \c{at(0)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa back(), at(), operator[]()
*/

/*!
    \fn QChar QString::back() const
    \since 5.10

    Returns the last character in the string.
    Same as \c{at(size() - 1)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa front(), at(), operator[]()
*/

/*!
    \fn QChar &QString::front()
    \since 5.10

    Returns a reference to the first character in the string.
    Same as \c{operator[](0)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa back(), at(), operator[]()
*/

/*!
    \fn QChar &QString::back()
    \since 5.10

    Returns a reference to the last character in the string.
    Same as \c{operator[](size() - 1)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa front(), at(), operator[]()
*/

/*!
    \fn void QString::truncate(qsizetype position)

    Truncates the string at the given \a position index.

    If the specified \a position index is beyond the end of the
    string, nothing happens.

    Example:

    \snippet qstring/main.cpp 83

    If \a position is negative, it is equivalent to passing zero.

    \sa chop(), resize(), first(), QStringView::truncate()
*/

void QString::truncate(qsizetype pos)
{
    if (pos < size())
        resize(pos);
}


/*!
    Removes \a n characters from the end of the string.

    If \a n is greater than or equal to size(), the result is an
    empty string; if \a n is negative, it is equivalent to passing zero.

    Example:
    \snippet qstring/main.cpp 15

    If you want to remove characters from the \e beginning of the
    string, use remove() instead.

    \sa truncate(), resize(), remove(), QStringView::chop()
*/
void QString::chop(qsizetype n)
{
    if (n > 0)
        resize(d.size - n);
}

/*!
    Sets every character in the string to character \a ch. If \a size
    is different from -1 (default), the string is resized to \a
    size beforehand.

    Example:

    \snippet qstring/main.cpp 21

    \sa resize()
*/

QString& QString::fill(QChar ch, qsizetype size)
{
    resize(size < 0 ? d.size : size);
    if (d.size) {
        QChar *i = (QChar*)d.data() + d.size;
        QChar *b = (QChar*)d.data();
        while (i != b)
           *--i = ch;
    }
    return *this;
}

/*!
    \fn qsizetype QString::length() const

    Returns the number of characters in this string.  Equivalent to
    size().

    \sa resize()
*/

/*!
    \fn qsizetype QString::size() const

    Returns the number of characters in this string.

    The last character in the string is at position size() - 1.

    Example:
    \snippet qstring/main.cpp 58

    \sa isEmpty(), resize()
*/

/*! \fn bool QString::isNull() const

    Returns \c true if this string is null; otherwise returns \c false.

    Example:

    \snippet qstring/main.cpp 28

    Qt makes a distinction between null strings and empty strings for
    historical reasons. For most applications, what matters is
    whether or not a string contains any data, and this can be
    determined using the isEmpty() function.

    \sa isEmpty()
*/

/*! \fn bool QString::isEmpty() const

    Returns \c true if the string has no characters; otherwise returns
    \c false.

    Example:

    \snippet qstring/main.cpp 27

    \sa size()
*/

/*! \fn QString &QString::operator+=(const QString &other)

    Appends the string \a other onto the end of this string and
    returns a reference to this string.

    Example:

    \snippet qstring/main.cpp 84

    This operation is typically very fast (\l{constant time}),
    because QString preallocates extra space at the end of the string
    data so it can grow without reallocating the entire string each
    time.

    \sa append(), prepend()
*/

/*! \fn QString &QString::operator+=(QLatin1String str)

    \overload operator+=()

    Appends the Latin-1 string \a str to this string.
*/

/*! \fn QString &QString::operator+=(const QByteArray &ba)

    \overload operator+=()

    Appends the byte array \a ba to this string. The byte array is converted
    to Unicode using the fromUtf8() function. If any NUL characters ('\\0')
    are embedded in the \a ba byte array, they will be included in the
    transformation.

    You can disable this function by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn QString &QString::operator+=(const char *str)

    \overload operator+=()

    Appends the string \a str to this string. The const char pointer
    is converted to Unicode using the fromUtf8() function.

    You can disable this function by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*! \fn QString &QString::operator+=(QStringView str)
    \since 6.0
    \overload operator+=()

    Appends the string view \a str to this string.
*/

/*! \fn QString &QString::operator+=(QChar ch)

    \overload operator+=()

    Appends the character \a ch to the string.
*/

/*!
    \fn bool QString::operator==(const char *s1, const QString &s2)

    \overload operator==()

    Returns \c true if \a s1 is equal to \a s2; otherwise returns \c false.
    Note that no string is equal to \a s1 being 0.

    Equivalent to \c {s1 != 0 && compare(s1, s2) == 0}.
*/

/*!
    \fn bool QString::operator!=(const char *s1, const QString &s2)

    Returns \c true if \a s1 is not equal to \a s2; otherwise returns
    \c false.

    For \a s1 != 0, this is equivalent to \c {compare(} \a s1, \a s2
    \c {) != 0}. Note that no string is equal to \a s1 being 0.
*/

/*!
    \fn bool QString::operator<(const char *s1, const QString &s2)

    Returns \c true if \a s1 is lexically less than \a s2; otherwise
    returns \c false.  For \a s1 != 0, this is equivalent to \c
    {compare(s1, s2) < 0}.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator<=(const char *s1, const QString &s2)

    Returns \c true if \a s1 is lexically less than or equal to \a s2;
    otherwise returns \c false.  For \a s1 != 0, this is equivalent to \c
    {compare(s1, s2) <= 0}.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator>(const char *s1, const QString &s2)

    Returns \c true if \a s1 is lexically greater than \a s2; otherwise
    returns \c false.  Equivalent to \c {compare(s1, s2) > 0}.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QString::operator>=(const char *s1, const QString &s2)

    Returns \c true if \a s1 is lexically greater than or equal to \a s2;
    otherwise returns \c false.  For \a s1 != 0, this is equivalent to \c
    {compare(s1, s2) >= 0}.

    \sa {Comparing Strings}
*/

/*!
    \fn const QString operator+(const QString &s1, const QString &s2)
    \relates QString

    Returns a string which is the result of concatenating \a s1 and \a
    s2.
*/

/*!
    \fn const QString operator+(const QString &s1, const char *s2)
    \relates QString

    Returns a string which is the result of concatenating \a s1 and \a
    s2 (\a s2 is converted to Unicode using the QString::fromUtf8()
    function).

    \sa QString::fromUtf8()
*/

/*!
    \fn const QString operator+(const char *s1, const QString &s2)
    \relates QString

    Returns a string which is the result of concatenating \a s1 and \a
    s2 (\a s1 is converted to Unicode using the QString::fromUtf8()
    function).

    \sa QString::fromUtf8()
*/

/*!
    \fn int QString::compare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs)
    \since 4.2

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    If \a cs is Qt::CaseSensitive, the comparison is case sensitive;
    otherwise the comparison is case insensitive.

    Case sensitive comparison is based exclusively on the numeric
    Unicode values of the characters and is very fast, but is not what
    a human would expect.  Consider sorting user-visible strings with
    localeAwareCompare().

    \snippet qstring/main.cpp 16

    \sa operator==(), operator<(), operator>(), {Comparing Strings}
*/

/*!
    \fn int QString::compare(const QString &s1, QLatin1String s2, Qt::CaseSensitivity cs)
    \since 4.2
    \overload compare()

    Performs a comparison of \a s1 and \a s2, using the case
    sensitivity setting \a cs.
*/

/*!
    \fn int QString::compare(QLatin1String s1, const QString &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)

    \since 4.2
    \overload compare()

    Performs a comparison of \a s1 and \a s2, using the case
    sensitivity setting \a cs.
*/

/*!
    \fn int QString::compare(QStringView s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \since 5.12
    \overload compare()

    Performs a comparison of this with \a s, using the case
    sensitivity setting \a cs.
*/

/*!
    \fn int QString::compare(QChar ch, Qt::CaseSensitivity cs = Qt::CaseSensitive) const

    \since 5.14
    \overload compare()

    Performs a comparison of this with \a ch, using the case
    sensitivity setting \a cs.
*/

/*!
    \overload compare()
    \since 4.2

    Lexically compares this string with the \a other string and
    returns an integer less than, equal to, or greater than zero if
    this string is less than, equal to, or greater than the other
    string.

    Same as compare(*this, \a other, \a cs).
*/
int QString::compare(const QString &other, Qt::CaseSensitivity cs) const noexcept
{
    return QtPrivate::compareStrings(*this, other, cs);
}

/*!
    \internal
    \since 4.5
*/
int QString::compare_helper(const QChar *data1, qsizetype length1, const QChar *data2, qsizetype length2,
                            Qt::CaseSensitivity cs) noexcept
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(length2 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    Q_ASSERT(data2 || length2 == 0);
    return QtPrivate::compareStrings(QStringView(data1, length1), QStringView(data2, length2), cs);
}

/*!
    \overload compare()
    \since 4.2

    Same as compare(*this, \a other, \a cs).
*/
int QString::compare(QLatin1String other, Qt::CaseSensitivity cs) const noexcept
{
    return QtPrivate::compareStrings(*this, other, cs);
}

/*!
    \internal
    \since 5.0
*/
int QString::compare_helper(const QChar *data1, qsizetype length1, const char *data2, qsizetype length2,
                            Qt::CaseSensitivity cs) noexcept
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    if (!data2)
        return length1;
    if (Q_UNLIKELY(length2 < 0))
        length2 = qsizetype(strlen(data2));
    return QtPrivate::compareStrings(QStringView(data1, length1),
                                     QUtf8StringView(data2, length2), cs);
}

/*!
  \fn int QString::compare(const QString &s1, QStringView s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
  \overload compare()
*/

/*!
  \fn int QString::compare(QStringView s1, const QString &s2, Qt::CaseSensitivity cs = Qt::CaseSensitive)
  \overload compare()
*/

/*!
    \internal
    \since 4.5
*/
int QLatin1String::compare_helper(const QChar *data1, qsizetype length1, QLatin1String s2,
                                  Qt::CaseSensitivity cs) noexcept
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    return QtPrivate::compareStrings(QStringView(data1, length1), s2, cs);
}

/*!
    \fn int QString::localeAwareCompare(const QString & s1, const QString & s2)

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa compare(), QLocale, {Comparing Strings}
*/

/*!
    \fn int QString::localeAwareCompare(QStringView other) const
    \since 6.0
    \overload localeAwareCompare()

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    Same as \c {localeAwareCompare(*this, other)}.

    \sa {Comparing Strings}
*/

/*!
    \fn int QString::localeAwareCompare(QStringView s1, QStringView s2)
    \since 6.0
    \overload localeAwareCompare()

    Compares \a s1 with \a s2 and returns an integer less than, equal
    to, or greater than zero if \a s1 is less than, equal to, or
    greater than \a s2.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    \sa {Comparing Strings}
*/


#if !defined(CSTR_LESS_THAN)
#define CSTR_LESS_THAN    1
#define CSTR_EQUAL        2
#define CSTR_GREATER_THAN 3
#endif

/*!
    \overload localeAwareCompare()

    Compares this string with the \a other string and returns an
    integer less than, equal to, or greater than zero if this string
    is less than, equal to, or greater than the \a other string.

    The comparison is performed in a locale- and also
    platform-dependent manner. Use this function to present sorted
    lists of strings to the user.

    Same as \c {localeAwareCompare(*this, other)}.

    \sa {Comparing Strings}
*/
int QString::localeAwareCompare(const QString &other) const
{
    return localeAwareCompare_helper(constData(), length(), other.constData(), other.length());
}

#if QT_CONFIG(icu)
namespace {
class GenerationalCollator
{
    QCollator theCollator;
    int generation = QLocalePrivate::s_generation.loadRelaxed();
public:
    QCollator &collator()
    {
        int currentGeneration = QLocalePrivate::s_generation.loadRelaxed();
        if (Q_UNLIKELY(generation != currentGeneration)) {
            // reinitialize the collator
            generation = currentGeneration;
            theCollator = QCollator();
        }
        return theCollator;
    }
};
}
Q_GLOBAL_STATIC(QThreadStorage<GenerationalCollator>, defaultCollator)
#endif

/*!
    \internal
    \since 4.5
*/
int QString::localeAwareCompare_helper(const QChar *data1, qsizetype length1,
                                       const QChar *data2, qsizetype length2)
{
    Q_ASSERT(length1 >= 0);
    Q_ASSERT(data1 || length1 == 0);
    Q_ASSERT(length2 >= 0);
    Q_ASSERT(data2 || length2 == 0);

    // do the right thing for null and empty
    if (length1 == 0 || length2 == 0)
        return QtPrivate::compareStrings(QStringView(data1, length1), QStringView(data2, length2),
                               Qt::CaseSensitive);

#if QT_CONFIG(icu)
    if (!defaultCollator()->hasLocalData())
        defaultCollator()->setLocalData(GenerationalCollator());
    return defaultCollator()->localData().collator().compare(data1, length1, data2, length2);
#else
    const QString lhs = QString::fromRawData(data1, length1).normalized(QString::NormalizationForm_C);
    const QString rhs = QString::fromRawData(data2, length2).normalized(QString::NormalizationForm_C);
#  if defined(Q_OS_WIN)
    int res = CompareStringEx(LOCALE_NAME_USER_DEFAULT, 0, (LPWSTR)lhs.constData(), lhs.length(), (LPWSTR)rhs.constData(), rhs.length(), NULL, NULL, 0);

    switch (res) {
    case CSTR_LESS_THAN:
        return -1;
    case CSTR_GREATER_THAN:
        return 1;
    default:
        return 0;
    }
#  elif defined (Q_OS_DARWIN)
    // Use CFStringCompare for comparing strings on Mac. This makes Qt order
    // strings the same way as native applications do, and also respects
    // the "Order for sorted lists" setting in the International preferences
    // panel.
    const CFStringRef thisString =
        CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
            reinterpret_cast<const UniChar *>(lhs.constData()), lhs.length(), kCFAllocatorNull);
    const CFStringRef otherString =
        CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
            reinterpret_cast<const UniChar *>(rhs.constData()), rhs.length(), kCFAllocatorNull);

    const int result = CFStringCompare(thisString, otherString, kCFCompareLocalized);
    CFRelease(thisString);
    CFRelease(otherString);
    return result;
#  elif defined(Q_OS_UNIXLIKE)
    // declared in <string.h> (no better than QtPrivate::compareStrings() on Android, sadly)
    return strcoll(lhs.toLocal8Bit().constData(), rhs.toLocal8Bit().constData());
#  else
#     error "This case shouldn't happen"
    return QtPrivate::compareStrings(lhs, rhs, Qt::CaseSensitive);
#  endif
#endif // !QT_CONFIG(icu)
}


/*!
    \fn const QChar *QString::unicode() const

    Returns a Unicode representation of the string.
    The result remains valid until the string is modified.

    \note The returned string may not be '\\0'-terminated.
    Use size() to determine the length of the array.

    \sa utf16(), fromRawData()
*/

/*!
    \fn const ushort *QString::utf16() const

    Returns the QString as a '\\0\'-terminated array of unsigned
    shorts. The result remains valid until the string is modified.

    The returned string is in host byte order.

    \sa unicode()
*/

const ushort *QString::utf16() const
{
    if (!d->isMutable()) {
        // ensure '\0'-termination for ::fromRawData strings
        const_cast<QString*>(this)->reallocData(d.size, QArrayData::KeepSize);
    }
    return reinterpret_cast<const ushort *>(d.data());
}

/*!
    Returns a string of size \a width that contains this string
    padded by the \a fill character.

    If \a truncate is \c false and the size() of the string is more than
    \a width, then the returned string is a copy of the string.

    \snippet qstring/main.cpp 32

    If \a truncate is \c true and the size() of the string is more than
    \a width, then any characters in a copy of the string after
    position \a width are removed, and the copy is returned.

    \snippet qstring/main.cpp 33

    \sa rightJustified()
*/

QString QString::leftJustified(qsizetype width, QChar fill, bool truncate) const
{
    QString result;
    qsizetype len = length();
    qsizetype padlen = width - len;
    if (padlen > 0) {
        result.resize(len+padlen);
        if (len)
            memcpy(result.d.data(), d.data(), sizeof(QChar)*len);
        QChar *uc = (QChar*)result.d.data() + len;
        while (padlen--)
           * uc++ = fill;
    } else {
        if (truncate)
            result = left(width);
        else
            result = *this;
    }
    return result;
}

/*!
    Returns a string of size() \a width that contains the \a fill
    character followed by the string. For example:

    \snippet qstring/main.cpp 49

    If \a truncate is \c false and the size() of the string is more than
    \a width, then the returned string is a copy of the string.

    If \a truncate is true and the size() of the string is more than
    \a width, then the resulting string is truncated at position \a
    width.

    \snippet qstring/main.cpp 50

    \sa leftJustified()
*/

QString QString::rightJustified(qsizetype width, QChar fill, bool truncate) const
{
    QString result;
    qsizetype len = length();
    qsizetype padlen = width - len;
    if (padlen > 0) {
        result.resize(len+padlen);
        QChar *uc = (QChar*)result.d.data();
        while (padlen--)
           * uc++ = fill;
        if (len)
          memcpy(static_cast<void *>(uc), static_cast<const void *>(d.data()), sizeof(QChar)*len);
    } else {
        if (truncate)
            result = left(width);
        else
            result = *this;
    }
    return result;
}

/*!
    \fn QString QString::toLower() const

    Returns a lowercase copy of the string.

    \snippet qstring/main.cpp 75

    The case conversion will always happen in the 'C' locale. For
    locale-dependent case folding use QLocale::toLower()

    \sa toUpper(), QLocale::toLower()
*/

namespace QUnicodeTables {
/*
    \internal
    Converts the \a str string starting from the position pointed to by the \a
    it iterator, using the Unicode case traits \c Traits, and returns the
    result. The input string must not be empty (the convertCase function below
    guarantees that).

    The string type \c{T} is also a template and is either \c{const QString} or
    \c{QString}. This function can do both copy-conversion and in-place
    conversion depending on the state of the \a str parameter:
    \list
       \li \c{T} is \c{const QString}: copy-convert
       \li \c{T} is \c{QString} and its refcount != 1: copy-convert
       \li \c{T} is \c{QString} and its refcount == 1: in-place convert
    \endlist

    In copy-convert mode, the local variable \c{s} is detached from the input
    \a str. In the in-place convert mode, \a str is in moved-from state and
    \c{s} contains the only copy of the string, without reallocation (thus,
    \a it is still valid).

    There is one pathological case left: when the in-place conversion needs to
    reallocate memory to grow the buffer. In that case, we need to adjust the \a
    it pointer.
 */
template <typename T>
Q_NEVER_INLINE
static QString detachAndConvertCase(T &str, QStringIterator it, QUnicodeTables::Case which)
{
    Q_ASSERT(!str.isEmpty());
    QString s = std::move(str);         // will copy if T is const QString
    QChar *pp = s.begin() + it.index(); // will detach if necessary

    do {
        const auto folded = fullConvertCase(it.next(), which);
        if (Q_UNLIKELY(folded.size() > 1)) {
            if (folded.chars[0] == *pp && folded.size() == 2) {
                // special case: only second actually changed (e.g. surrogate pairs),
                // avoid slow case
                ++pp;
                *pp++ = folded.chars[1];
            } else {
                // slow path: the string is growing
                qsizetype inpos = it.index() - 1;
                qsizetype outpos = pp - s.constBegin();

                s.replace(outpos, 1, reinterpret_cast<const QChar *>(folded.data()), folded.size());
                pp = const_cast<QChar *>(s.constBegin()) + outpos + folded.size();

                // Adjust the input iterator if we are performing an in-place conversion
                if constexpr (!std::is_const<T>::value)
                    it = QStringIterator(s.constBegin(), inpos + folded.size(), s.constEnd());
            }
        } else {
            *pp++ = folded.chars[0];
        }
    } while (it.hasNext());

    return s;
}

template <typename T>
static QString convertCase(T &str, QUnicodeTables::Case which)
{
    const QChar *p = str.constBegin();
    const QChar *e = p + str.size();

    // this avoids out of bounds check in the loop
    while (e != p && e[-1].isHighSurrogate())
        --e;

    QStringIterator it(p, e);
    while (it.hasNext()) {
        const char32_t uc = it.next();
        if (qGetProp(uc)->cases[which].diff) {
            it.recede();
            return detachAndConvertCase(str, it, which);
        }
    }
    return std::move(str);
}
} // namespace QUnicodeTables

QString QString::toLower_helper(const QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::LowerCase);
}

QString QString::toLower_helper(QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::LowerCase);
}

/*!
    \fn QString QString::toCaseFolded() const

    Returns the case folded equivalent of the string. For most Unicode
    characters this is the same as toLower().
*/

QString QString::toCaseFolded_helper(const QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::CaseFold);
}

QString QString::toCaseFolded_helper(QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::CaseFold);
}

/*!
    \fn QString QString::toUpper() const

    Returns an uppercase copy of the string.

    \snippet qstring/main.cpp 81

    The case conversion will always happen in the 'C' locale. For
    locale-dependent case folding use QLocale::toUpper()

    \sa toLower(), QLocale::toLower()
*/

QString QString::toUpper_helper(const QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::UpperCase);
}

QString QString::toUpper_helper(QString &str)
{
    return QUnicodeTables::convertCase(str, QUnicodeTables::UpperCase);
}

/*!
    \since 5.5

    Safely builds a formatted string from the format string \a cformat
    and an arbitrary list of arguments.

    The format string supports the conversion specifiers, length modifiers,
    and flags provided by printf() in the standard C++ library. The \a cformat
    string and \c{%s} arguments must be UTF-8 encoded.

    \note The \c{%lc} escape sequence expects a unicode character of type
    \c char16_t, or \c ushort (as returned by QChar::unicode()).
    The \c{%ls} escape sequence expects a pointer to a zero-terminated array
    of unicode characters of type \c char16_t, or ushort (as returned by
    QString::utf16()). This is at odds with the printf() in the standard C++
    library, which defines \c {%lc} to print a wchar_t and \c{%ls} to print
    a \c{wchar_t*}, and might also produce compiler warnings on platforms
    where the size of \c {wchar_t} is not 16 bits.

    \warning We do not recommend using QString::asprintf() in new Qt
    code. Instead, consider using QTextStream or arg(), both of
    which support Unicode strings seamlessly and are type-safe.
    Here is an example that uses QTextStream:

    \snippet qstring/main.cpp 64

    For \l {QObject::tr()}{translations}, especially if the strings
    contains more than one escape sequence, you should consider using
    the arg() function instead. This allows the order of the
    replacements to be controlled by the translator.

    \sa arg()
*/

QString QString::asprintf(const char *cformat, ...)
{
    va_list ap;
    va_start(ap, cformat);
    const QString s = vasprintf(cformat, ap);
    va_end(ap);
    return s;
}

static void append_utf8(QString &qs, const char *cs, qsizetype len)
{
    const qsizetype oldSize = qs.size();
    qs.resize(oldSize + len);
    const QChar *newEnd = QUtf8::convertToUnicode(qs.data() + oldSize, QByteArrayView(cs, len));
    qs.resize(newEnd - qs.constData());
}

static uint parse_flag_characters(const char * &c) noexcept
{
    uint flags = QLocaleData::ZeroPadExponent;
    while (true) {
        switch (*c) {
        case '#':
            flags |= QLocaleData::ShowBase | QLocaleData::AddTrailingZeroes
                    | QLocaleData::ForcePoint;
            break;
        case '0': flags |= QLocaleData::ZeroPadded; break;
        case '-': flags |= QLocaleData::LeftAdjusted; break;
        case ' ': flags |= QLocaleData::BlankBeforePositive; break;
        case '+': flags |= QLocaleData::AlwaysShowSign; break;
        case '\'': flags |= QLocaleData::GroupDigits; break;
        default: return flags;
        }
        ++c;
    }
}

static int parse_field_width(const char * &c)
{
    Q_ASSERT(qIsDigit(*c));

    // can't be negative - started with a digit
    // contains at least one digit
    const char *endp;
    bool ok;
    const qulonglong result = qstrtoull(c, &endp, 10, &ok);
    c = endp;
    while (qIsDigit(*c)) // preserve Qt 5.5 behavior of consuming all digits, no matter how many
        ++c;
    return ok && result < qulonglong(std::numeric_limits<int>::max()) ? int(result) : 0;
}

enum LengthMod { lm_none, lm_hh, lm_h, lm_l, lm_ll, lm_L, lm_j, lm_z, lm_t };

static inline bool can_consume(const char * &c, char ch) noexcept
{
    if (*c == ch) {
        ++c;
        return true;
    }
    return false;
}

static LengthMod parse_length_modifier(const char * &c) noexcept
{
    switch (*c++) {
    case 'h': return can_consume(c, 'h') ? lm_hh : lm_h;
    case 'l': return can_consume(c, 'l') ? lm_ll : lm_l;
    case 'L': return lm_L;
    case 'j': return lm_j;
    case 'z':
    case 'Z': return lm_z;
    case 't': return lm_t;
    }
    --c; // don't consume *c - it wasn't a flag
    return lm_none;
}

/*!
    \fn QString QString::vasprintf(const char *cformat, va_list ap)
    \since 5.5

    Equivalent method to asprintf(), but takes a va_list \a ap
    instead a list of variable arguments. See the asprintf()
    documentation for an explanation of \a cformat.

    This method does not call the va_end macro, the caller
    is responsible to call va_end on \a ap.

    \sa asprintf()
*/

QString QString::vasprintf(const char *cformat, va_list ap)
{
    if (!cformat || !*cformat) {
        // Qt 1.x compat
        return fromLatin1("");
    }

    // Parse cformat

    QString result;
    const char *c = cformat;
    for (;;) {
        // Copy non-escape chars to result
        const char *cb = c;
        while (*c != '\0' && *c != '%')
            c++;
        append_utf8(result, cb, qsizetype(c - cb));

        if (*c == '\0')
            break;

        // Found '%'
        const char *escape_start = c;
        ++c;

        if (*c == '\0') {
            result.append(QLatin1Char('%')); // a % at the end of the string - treat as non-escape text
            break;
        }
        if (*c == '%') {
            result.append(QLatin1Char('%')); // %%
            ++c;
            continue;
        }

        uint flags = parse_flag_characters(c);

        if (*c == '\0') {
            result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
            break;
        }

        // Parse field width
        int width = -1; // -1 means unspecified
        if (qIsDigit(*c)) {
            width = parse_field_width(c);
        } else if (*c == '*') { // can't parse this in another function, not portably, at least
            width = va_arg(ap, int);
            if (width < 0)
                width = -1; // treat all negative numbers as unspecified
            ++c;
        }

        if (*c == '\0') {
            result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
            break;
        }

        // Parse precision
        int precision = -1; // -1 means unspecified
        if (*c == '.') {
            ++c;
            precision = 0;
            if (qIsDigit(*c)) {
                precision = parse_field_width(c);
            } else if (*c == '*') { // can't parse this in another function, not portably, at least
                precision = va_arg(ap, int);
                if (precision < 0)
                    precision = -1; // treat all negative numbers as unspecified
                ++c;
            }
        }

        if (*c == '\0') {
            result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
            break;
        }

        const LengthMod length_mod = parse_length_modifier(c);

        if (*c == '\0') {
            result.append(QLatin1String(escape_start)); // incomplete escape, treat as non-escape text
            break;
        }

        // Parse the conversion specifier and do the conversion
        QString subst;
        switch (*c) {
            case 'd':
            case 'i': {
                qint64 i;
                switch (length_mod) {
                    case lm_none: i = va_arg(ap, int); break;
                    case lm_hh: i = va_arg(ap, int); break;
                    case lm_h: i = va_arg(ap, int); break;
                    case lm_l: i = va_arg(ap, long int); break;
                    case lm_ll: i = va_arg(ap, qint64); break;
                    case lm_j: i = va_arg(ap, long int); break;

                    /* ptrdiff_t actually, but it should be the same for us */
                    case lm_z: i = va_arg(ap, qsizetype); break;
                    case lm_t: i = va_arg(ap, qsizetype); break;
                    default: i = 0; break;
                }
                subst = QLocaleData::c()->longLongToString(i, precision, 10, width, flags);
                ++c;
                break;
            }
            case 'o':
            case 'u':
            case 'x':
            case 'X': {
                quint64 u;
                switch (length_mod) {
                    case lm_none: u = va_arg(ap, uint); break;
                    case lm_hh: u = va_arg(ap, uint); break;
                    case lm_h: u = va_arg(ap, uint); break;
                    case lm_l: u = va_arg(ap, ulong); break;
                    case lm_ll: u = va_arg(ap, quint64); break;
                    case lm_t: u = va_arg(ap, size_t); break;
                    case lm_z: u = va_arg(ap, size_t); break;
                    default: u = 0; break;
                }

                if (qIsUpper(*c))
                    flags |= QLocaleData::CapitalEorX;

                int base = 10;
                switch (qToLower(*c)) {
                    case 'o':
                        base = 8; break;
                    case 'u':
                        base = 10; break;
                    case 'x':
                        base = 16; break;
                    default: break;
                }
                subst = QLocaleData::c()->unsLongLongToString(u, precision, base, width, flags);
                ++c;
                break;
            }
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
            case 'A':
            case 'a': {
                double d;
                if (length_mod == lm_L)
                    d = va_arg(ap, long double); // not supported - converted to a double
                else
                    d = va_arg(ap, double);

                if (qIsUpper(*c))
                    flags |= QLocaleData::CapitalEorX;

                QLocaleData::DoubleForm form = QLocaleData::DFDecimal;
                switch (qToLower(*c)) {
                    case 'e': form = QLocaleData::DFExponent; break;
                    case 'a':                             // not supported - decimal form used instead
                    case 'f': form = QLocaleData::DFDecimal; break;
                    case 'g': form = QLocaleData::DFSignificantDigits; break;
                    default: break;
                }
                subst = QLocaleData::c()->doubleToString(d, precision, form, width, flags);
                ++c;
                break;
            }
            case 'c': {
                if (length_mod == lm_l)
                    subst = QChar::fromUcs2(va_arg(ap, int));
                else
                    subst = QLatin1Char((uchar) va_arg(ap, int));
                ++c;
                break;
            }
            case 's': {
                if (length_mod == lm_l) {
                    const ushort *buff = va_arg(ap, const ushort*);
                    const ushort *ch = buff;
                    while (precision != 0 && *ch != 0) {
                        ++ch;
                        --precision;
                    }
                    subst.setUtf16(buff, ch - buff);
                } else if (precision == -1) {
                    subst = QString::fromUtf8(va_arg(ap, const char*));
                } else {
                    const char *buff = va_arg(ap, const char*);
                    subst = QString::fromUtf8(buff, qstrnlen(buff, precision));
                }
                ++c;
                break;
            }
            case 'p': {
                void *arg = va_arg(ap, void*);
                const quint64 i = reinterpret_cast<quintptr>(arg);
                flags |= QLocaleData::ShowBase;
                subst = QLocaleData::c()->unsLongLongToString(i, precision, 16, width, flags);
                ++c;
                break;
            }
            case 'n':
                switch (length_mod) {
                    case lm_hh: {
                        signed char *n = va_arg(ap, signed char*);
                        *n = result.length();
                        break;
                    }
                    case lm_h: {
                        short int *n = va_arg(ap, short int*);
                        *n = result.length();
                            break;
                    }
                    case lm_l: {
                        long int *n = va_arg(ap, long int*);
                        *n = result.length();
                        break;
                    }
                    case lm_ll: {
                        qint64 *n = va_arg(ap, qint64*);
                        *n = result.length();
                        break;
                    }
                    default: {
                        int *n = va_arg(ap, int*);
                        *n = result.length();
                        break;
                    }
                }
                ++c;
                break;

            default: // bad escape, treat as non-escape text
                for (const char *cc = escape_start; cc != c; ++cc)
                    result.append(QLatin1Char(*cc));
                continue;
        }

        if (flags & QLocaleData::LeftAdjusted)
            result.append(subst.leftJustified(width));
        else
            result.append(subst.rightJustified(width));
    }

    return result;
}

/*!
    Returns the string converted to a \c{long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toLongLong()

    Example:

    \snippet qstring/main.cpp 74

    This function ignores leading and trailing whitespace.

    \sa number(), toULongLong(), toInt(), QLocale::toLongLong()
*/

qint64 QString::toLongLong(bool *ok, int base) const
{
    return toIntegral_helper<qlonglong>(*this, ok, base);
}

qlonglong QString::toIntegral_helper(QStringView string, bool *ok, int base)
{
#if defined(QT_CHECK_RANGE)
    if (base != 0 && (base < 2 || base > 36)) {
        qWarning("QString::toULongLong: Invalid base (%d)", base);
        base = 10;
    }
#endif

    return QLocaleData::c()->stringToLongLong(string, base, ok, QLocale::RejectGroupSeparator);
}


/*!
    Returns the string converted to an \c{unsigned long long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toULongLong()

    Example:

    \snippet qstring/main.cpp 79

    This function ignores leading and trailing whitespace.

    \sa number(), toLongLong(), QLocale::toULongLong()
*/

quint64 QString::toULongLong(bool *ok, int base) const
{
    return toIntegral_helper<qulonglong>(*this, ok, base);
}

qulonglong QString::toIntegral_helper(QStringView string, bool *ok, uint base)
{
#if defined(QT_CHECK_RANGE)
    if (base != 0 && (base < 2 || base > 36)) {
        qWarning("QString::toULongLong: Invalid base (%d)", base);
        base = 10;
    }
#endif

    return QLocaleData::c()->stringToUnsLongLong(string, base, ok, QLocale::RejectGroupSeparator);
}

/*!
    \fn long QString::toLong(bool *ok, int base) const

    Returns the string converted to a \c long using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toLongLong()

    Example:

    \snippet qstring/main.cpp 73

    This function ignores leading and trailing whitespace.

    \sa number(), toULong(), toInt(), QLocale::toInt()
*/

/*!
    \fn ulong QString::toULong(bool *ok, int base) const

    Returns the string converted to an \c{unsigned long} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toULongLong()

    Example:

    \snippet qstring/main.cpp 78

    This function ignores leading and trailing whitespace.

    \sa number(), QLocale::toUInt()
*/

/*!
    \fn int QString::toInt(bool *ok, int base) const
    Returns the string converted to an \c int using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toInt()

    Example:

    \snippet qstring/main.cpp 72

    This function ignores leading and trailing whitespace.

    \sa number(), toUInt(), toDouble(), QLocale::toInt()
*/

/*!
    \fn uint QString::toUInt(bool *ok, int base) const
    Returns the string converted to an \c{unsigned int} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toUInt()

    Example:

    \snippet qstring/main.cpp 77

    This function ignores leading and trailing whitespace.

    \sa number(), toInt(), QLocale::toUInt()
*/

/*!
    \fn short QString::toShort(bool *ok, int base) const

    Returns the string converted to a \c short using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toShort()

    Example:

    \snippet qstring/main.cpp 76

    This function ignores leading and trailing whitespace.

    \sa number(), toUShort(), toInt(), QLocale::toShort()
*/

/*!
    \fn ushort QString::toUShort(bool *ok, int base) const

    Returns the string converted to an \c{unsigned short} using base \a
    base, which is 10 by default and must be between 2 and 36, or 0.
    Returns 0 if the conversion fails.

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    If \a base is 0, the C language convention is used: If the string
    begins with "0x", base 16 is used; if the string begins with "0",
    base 8 is used; otherwise, base 10 is used.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toUShort()

    Example:

    \snippet qstring/main.cpp 80

    This function ignores leading and trailing whitespace.

    \sa number(), toShort(), QLocale::toUShort()
*/

/*!
    Returns the string converted to a \c double value.

    Returns an infinity if the conversion overflows or 0.0 if the
    conversion fails for other reasons (e.g. underflow).

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    \snippet qstring/main.cpp 66

    \warning The QString content may only contain valid numerical characters
    which includes the plus/minus sign, the character e used in scientific
    notation, and the decimal point. Including the unit or additional characters
    leads to a conversion error.

    \snippet qstring/main.cpp 67

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toDouble()

    \snippet qstring/main.cpp 68

    For historical reasons, this function does not handle
    thousands group separators. If you need to convert such numbers,
    use QLocale::toDouble().

    \snippet qstring/main.cpp 69

    This function ignores leading and trailing whitespace.

    \sa number(), QLocale::setDefault(), QLocale::toDouble(), trimmed()
*/

double QString::toDouble(bool *ok) const
{
    return QLocaleData::c()->stringToDouble(*this, ok, QLocale::RejectGroupSeparator);
}

/*!
    Returns the string converted to a \c float value.

    Returns an infinity if the conversion overflows or 0.0 if the
    conversion fails for other reasons (e.g. underflow).

    If \a ok is not \nullptr, failure is reported by setting *\a{ok}
    to \c false, and success by setting *\a{ok} to \c true.

    \warning The QString content may only contain valid numerical characters
    which includes the plus/minus sign, the character e used in scientific
    notation, and the decimal point. Including the unit or additional characters
    leads to a conversion error.

    The string conversion will always happen in the 'C' locale. For
    locale-dependent conversion use QLocale::toFloat()

    For historical reasons, this function does not handle
    thousands group separators. If you need to convert such numbers,
    use QLocale::toFloat().

    Example:

    \snippet qstring/main.cpp 71

    This function ignores leading and trailing whitespace.

    \sa number(), toDouble(), toInt(), QLocale::toFloat(), trimmed()
*/

float QString::toFloat(bool *ok) const
{
    return QLocaleData::convertDoubleToFloat(toDouble(ok), ok);
}

/*! \fn QString &QString::setNum(int n, int base)

    Sets the string to the printed value of \a n in the specified \a
    base, and returns a reference to the string.

    The base is 10 by default and must be between 2 and 36.

    \snippet qstring/main.cpp 56

   The formatting always uses QLocale::C, i.e., English/UnitedStates.
   To get a localized string representation of a number, use
   QLocale::toString() with the appropriate locale.

   \sa number()
*/

/*! \fn QString &QString::setNum(uint n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(long n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(ulong n, int base)

    \overload
*/

/*!
    \overload
*/
QString &QString::setNum(qlonglong n, int base)
{
    return *this = number(n, base);
}

/*!
    \overload
*/
QString &QString::setNum(qulonglong n, int base)
{
    return *this = number(n, base);
}

/*! \fn QString &QString::setNum(short n, int base)

    \overload
*/

/*! \fn QString &QString::setNum(ushort n, int base)

    \overload
*/

/*!
    \overload

    Sets the string to the printed value of \a n, formatted according to the
    given \a format and \a precision, and returns a reference to the string.

    \sa number(), QLocale::FloatingPointPrecisionOption, {Number Formats}
*/

QString &QString::setNum(double n, char format, int precision)
{
    return *this = number(n, format, precision);
}

/*!
    \fn QString &QString::setNum(float n, char format, int precision)
    \overload

    Sets the string to the printed value of \a n, formatted according
    to the given \a format and \a precision, and returns a reference
    to the string.

    The formatting always uses QLocale::C, i.e., English/UnitedStates.
    To get a localized string representation of a number, use
    QLocale::toString() with the appropriate locale.

    \sa number()
*/


/*!
    \fn QString QString::number(long n, int base)

    Returns a string equivalent of the number \a n according to the
    specified \a base.

    The base is 10 by default and must be between 2
    and 36. For bases other than 10, \a n is treated as an
    unsigned integer.

    The formatting always uses QLocale::C, i.e., English/UnitedStates.
    To get a localized string representation of a number, use
    QLocale::toString() with the appropriate locale.

    \snippet qstring/main.cpp 35

    \sa setNum()
*/

QString QString::number(long n, int base)
{
    return number(qlonglong(n), base);
}

/*!
  \fn QString QString::number(ulong n, int base)

    \overload
*/
QString QString::number(ulong n, int base)
{
    return number(qulonglong(n), base);
}

/*!
    \overload
*/
QString QString::number(int n, int base)
{
    return number(qlonglong(n), base);
}

/*!
    \overload
*/
QString QString::number(uint n, int base)
{
    return number(qulonglong(n), base);
}

/*!
    \overload
*/
QString QString::number(qlonglong n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (base < 2 || base > 36) {
        qWarning("QString::setNum: Invalid base (%d)", base);
        base = 10;
    }
#endif
    return QLocaleData::c()->longLongToString(n, -1, base);
}

/*!
    \overload
*/
QString QString::number(qulonglong n, int base)
{
#if defined(QT_CHECK_RANGE)
    if (base < 2 || base > 36) {
        qWarning("QString::setNum: Invalid base (%d)", base);
        base = 10;
    }
#endif
    return QLocaleData::c()->unsLongLongToString(n, -1, base);
}


/*!
    Returns a string representing the floating-point number \a n.

    Returns a string that represents \a n, formatted according to the specified
    \a format and \a precision.

    For formats with an exponent, the exponent will show its sign and have at
    least two digits, left-padding the exponent with zero if needed.

    \sa setNum(), QLocale::toString(), QLocale::FloatingPointPrecisionOption, {Number Formats}
*/
QString QString::number(double n, char format, int precision)
{
    QLocaleData::DoubleForm form = QLocaleData::DFDecimal;
    uint flags = QLocaleData::ZeroPadExponent;

    if (qIsUpper(format))
        flags |= QLocaleData::CapitalEorX;

    switch (qToLower(format)) {
        case 'f':
            form = QLocaleData::DFDecimal;
            break;
        case 'e':
            form = QLocaleData::DFExponent;
            break;
        case 'g':
            form = QLocaleData::DFSignificantDigits;
            break;
        default:
#if defined(QT_CHECK_RANGE)
            qWarning("QString::setNum: Invalid format char '%c'", format);
#endif
            break;
    }

    return QLocaleData::c()->doubleToString(n, precision, form, -1, flags);
}

namespace {
template<class ResultList, class StringSource>
static ResultList splitString(const StringSource &source, QStringView sep,
                              Qt::SplitBehavior behavior, Qt::CaseSensitivity cs)
{
    ResultList list;
    typename StringSource::size_type start = 0;
    typename StringSource::size_type end;
    typename StringSource::size_type extra = 0;
    while ((end = QtPrivate::findString(QStringView(source.constData(), source.size()), start + extra, sep, cs)) != -1) {
        if (start != end || behavior == Qt::KeepEmptyParts)
            list.append(source.mid(start, end - start));
        start = end + sep.size();
        extra = (sep.size() == 0 ? 1 : 0);
    }
    if (start != source.size() || behavior == Qt::KeepEmptyParts)
        list.append(source.mid(start));
    return list;
}

} // namespace

/*!
    Splits the string into substrings wherever \a sep occurs, and
    returns the list of those strings. If \a sep does not match
    anywhere in the string, split() returns a single-element list
    containing this string.

    \a cs specifies whether \a sep should be matched case
    sensitively or case insensitively.

    If \a behavior is Qt::SkipEmptyParts, empty entries don't
    appear in the result. By default, empty entries are kept.

    Example:

    \snippet qstring/main.cpp 62

    If \a sep is empty, split() returns an empty string, followed
    by each of the string's characters, followed by another empty string:

    \snippet qstring/main.cpp 62-empty

    To understand this behavior, recall that the empty string matches
    everywhere, so the above is qualitatively the same as:

    \snippet qstring/main.cpp 62-slashes

    \sa QStringList::join(), section()

    \since 5.14
*/
QStringList QString::split(const QString &sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return splitString<QStringList>(*this, sep, behavior, cs);
}

/*!
    \overload
    \since 5.14
*/
QStringList QString::split(QChar sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return splitString<QStringList>(*this, QStringView(&sep, 1), behavior, cs);
}

/*!
    \fn QList<QStringView> QStringView::split(QChar sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
    \fn QList<QStringView> QStringView::split(QStringView sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const


    Splits the view into substring views wherever \a sep occurs, and
    returns the list of those string views.

    See QString::split() for how \a sep, \a behavior and \a cs interact to form
    the result.

    \note All the returned views are valid as long as the data referenced by
    this string view is valid. Destroying the data will cause all views to
    become dangling.

    \since 6.0
*/
QList<QStringView> QStringView::split(QStringView sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return splitString<QList<QStringView>>(QStringView(*this), sep, behavior, cs);
}

QList<QStringView> QStringView::split(QChar sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return split(QStringView(&sep, 1), behavior, cs);
}

#if QT_CONFIG(regularexpression)
namespace {
template<class ResultList, typename String>
static ResultList splitString(const String &source, const QRegularExpression &re,
                              Qt::SplitBehavior behavior)
{
    ResultList list;
    if (!re.isValid()) {
        qWarning("QString::split: invalid QRegularExpression object");
        return list;
    }

    qsizetype start = 0;
    qsizetype end = 0;
    QRegularExpressionMatchIterator iterator = re.globalMatch(source);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        end = match.capturedStart();
        if (start != end || behavior == Qt::KeepEmptyParts)
            list.append(source.mid(start, end - start));
        start = match.capturedEnd();
    }

    if (start != source.size() || behavior == Qt::KeepEmptyParts)
        list.append(source.mid(start));

    return list;
}
} // namespace

/*!
    \overload
    \since 5.14

    Splits the string into substrings wherever the regular expression
    \a re matches, and returns the list of those strings. If \a re
    does not match anywhere in the string, split() returns a
    single-element list containing this string.

    Here is an example where we extract the words in a sentence
    using one or more whitespace characters as the separator:

    \snippet qstring/main.cpp 90

    Here is a similar example, but this time we use any sequence of
    non-word characters as the separator:

    \snippet qstring/main.cpp 91

    Here is a third example where we use a zero-length assertion,
    \b{\\b} (word boundary), to split the string into an
    alternating sequence of non-word and word tokens:

    \snippet qstring/main.cpp 92

    \sa QStringList::join(), section()
*/
QStringList QString::split(const QRegularExpression &re, Qt::SplitBehavior behavior) const
{
    return splitString<QStringList>(*this, re, behavior);
}

/*!
    \since 6.0

    Splits the string into substring views wherever the regular expression \a re
    matches, and returns the list of those strings. If \a re does not match
    anywhere in the string, split() returns a single-element list containing
    this string as view.

    \note The views in the returned list are sub-views of this view; as such,
    they reference the same data as it and only remain valid for as long as that
    data remains live.
*/
QList<QStringView> QStringView::split(const QRegularExpression &re, Qt::SplitBehavior behavior) const
{
    return splitString<QList<QStringView>>(*this, re, behavior);
}

#endif // QT_CONFIG(regularexpression)

/*!
    \enum QString::NormalizationForm

    This enum describes the various normalized forms of Unicode text.

    \value NormalizationForm_D  Canonical Decomposition
    \value NormalizationForm_C  Canonical Decomposition followed by Canonical Composition
    \value NormalizationForm_KD  Compatibility Decomposition
    \value NormalizationForm_KC  Compatibility Decomposition followed by Canonical Composition

    \sa normalized(),
        {https://www.unicode.org/reports/tr15/}{Unicode Standard Annex #15}
*/

/*!
    \since 4.5

    Returns a copy of this string repeated the specified number of \a times.

    If \a times is less than 1, an empty string is returned.

    Example:

    \snippet code/src_corelib_text_qstring.cpp 8
*/
QString QString::repeated(qsizetype times) const
{
    if (d.size == 0)
        return *this;

    if (times <= 1) {
        if (times == 1)
            return *this;
        return QString();
    }

    const qsizetype resultSize = times * d.size;

    QString result;
    result.reserve(resultSize);
    if (result.capacity() != resultSize)
        return QString(); // not enough memory

    memcpy(result.d.data(), d.data(), d.size * sizeof(QChar));

    qsizetype sizeSoFar = d.size;
    char16_t *end = result.d.data() + sizeSoFar;

    const qsizetype halfResultSize = resultSize >> 1;
    while (sizeSoFar <= halfResultSize) {
        memcpy(end, result.d.data(), sizeSoFar * sizeof(QChar));
        end += sizeSoFar;
        sizeSoFar <<= 1;
    }
    memcpy(end, result.d.data(), (resultSize - sizeSoFar) * sizeof(QChar));
    result.d.data()[resultSize] = '\0';
    result.d.size = resultSize;
    return result;
}

void qt_string_normalize(QString *data, QString::NormalizationForm mode, QChar::UnicodeVersion version, qsizetype from)
{
    {
        // check if it's fully ASCII first, because then we have no work
        auto start = reinterpret_cast<const char16_t *>(data->constData());
        const char16_t *p = start + from;
        if (isAscii_helper(p, p + data->length() - from))
            return;
        if (p > start + from)
            from = p - start - 1;        // need one before the non-ASCII to perform NFC
    }

    if (version == QChar::Unicode_Unassigned) {
        version = QChar::currentUnicodeVersion();
    } else if (int(version) <= NormalizationCorrectionsVersionMax) {
        const QString &s = *data;
        QChar *d = nullptr;
        for (int i = 0; i < NumNormalizationCorrections; ++i) {
            const NormalizationCorrection &n = uc_normalization_corrections[i];
            if (n.version > version) {
                qsizetype pos = from;
                if (QChar::requiresSurrogates(n.ucs4)) {
                    char16_t ucs4High = QChar::highSurrogate(n.ucs4);
                    char16_t ucs4Low = QChar::lowSurrogate(n.ucs4);
                    char16_t oldHigh = QChar::highSurrogate(n.old_mapping);
                    char16_t oldLow = QChar::lowSurrogate(n.old_mapping);
                    while (pos < s.length() - 1) {
                        if (s.at(pos).unicode() == ucs4High && s.at(pos + 1).unicode() == ucs4Low) {
                            if (!d)
                                d = data->data();
                            d[pos] = QChar(oldHigh);
                            d[++pos] = QChar(oldLow);
                        }
                        ++pos;
                    }
                } else {
                    while (pos < s.length()) {
                        if (s.at(pos).unicode() == n.ucs4) {
                            if (!d)
                                d = data->data();
                            d[pos] = QChar(n.old_mapping);
                        }
                        ++pos;
                    }
                }
            }
        }
    }

    if (normalizationQuickCheckHelper(data, mode, from, &from))
        return;

    decomposeHelper(data, mode < QString::NormalizationForm_KD, version, from);

    canonicalOrderHelper(data, version, from);

    if (mode == QString::NormalizationForm_D || mode == QString::NormalizationForm_KD)
        return;

    composeHelper(data, version, from);
}

/*!
    Returns the string in the given Unicode normalization \a mode,
    according to the given \a version of the Unicode standard.
*/
QString QString::normalized(QString::NormalizationForm mode, QChar::UnicodeVersion version) const
{
    QString copy = *this;
    qt_string_normalize(&copy, mode, version, 0);
    return copy;
}


struct ArgEscapeData
{
    int min_escape;            // lowest escape sequence number
    int occurrences;           // number of occurrences of the lowest escape sequence number
    int locale_occurrences;    // number of occurrences of the lowest escape sequence number that
                               // contain 'L'
    int escape_len;            // total length of escape sequences which will be replaced
};

static ArgEscapeData findArgEscapes(QStringView s)
{
    const QChar *uc_begin = s.begin();
    const QChar *uc_end = s.end();

    ArgEscapeData d;

    d.min_escape = INT_MAX;
    d.occurrences = 0;
    d.escape_len = 0;
    d.locale_occurrences = 0;

    const QChar *c = uc_begin;
    while (c != uc_end) {
        while (c != uc_end && c->unicode() != '%')
            ++c;

        if (c == uc_end)
            break;
        const QChar *escape_start = c;
        if (++c == uc_end)
            break;

        bool locale_arg = false;
        if (c->unicode() == 'L') {
            locale_arg = true;
            if (++c == uc_end)
                break;
        }

        int escape = c->digitValue();
        if (escape == -1)
            continue;

        ++c;

        if (c != uc_end) {
            int next_escape = c->digitValue();
            if (next_escape != -1) {
                escape = (10 * escape) + next_escape;
                ++c;
            }
        }

        if (escape > d.min_escape)
            continue;

        if (escape < d.min_escape) {
            d.min_escape = escape;
            d.occurrences = 0;
            d.escape_len = 0;
            d.locale_occurrences = 0;
        }

        ++d.occurrences;
        if (locale_arg)
            ++d.locale_occurrences;
        d.escape_len += c - escape_start;
    }
    return d;
}

static QString replaceArgEscapes(QStringView s, const ArgEscapeData &d, int field_width,
                                 QStringView arg, QStringView larg, QChar fillChar)
{
    const QChar *uc_begin = s.begin();
    const QChar *uc_end = s.end();

    int abs_field_width = qAbs(field_width);
    qsizetype result_len = s.length()
                     - d.escape_len
                     + (d.occurrences - d.locale_occurrences)
                     *qMax(abs_field_width, arg.length())
                     + d.locale_occurrences
                     *qMax(abs_field_width, larg.length());

    QString result(result_len, Qt::Uninitialized);
    QChar *result_buff = const_cast<QChar *>(result.unicode());

    QChar *rc = result_buff;
    const QChar *c = uc_begin;
    int repl_cnt = 0;
    while (c != uc_end) {
        /* We don't have to check if we run off the end of the string with c,
           because as long as d.occurrences > 0 we KNOW there are valid escape
           sequences. */

        const QChar *text_start = c;

        while (c->unicode() != '%')
            ++c;

        const QChar *escape_start = c++;

        bool locale_arg = false;
        if (c->unicode() == 'L') {
            locale_arg = true;
            ++c;
        }

        int escape = c->digitValue();
        if (escape != -1) {
            if (c + 1 != uc_end && (c + 1)->digitValue() != -1) {
                escape = (10 * escape) + (c + 1)->digitValue();
                ++c;
            }
        }

        if (escape != d.min_escape) {
            memcpy(rc, text_start, (c - text_start)*sizeof(QChar));
            rc += c - text_start;
        }
        else {
            ++c;

            memcpy(rc, text_start, (escape_start - text_start)*sizeof(QChar));
            rc += escape_start - text_start;

            uint pad_chars;
            if (locale_arg)
                pad_chars = qMax(abs_field_width, larg.length()) - larg.length();
            else
                pad_chars = qMax(abs_field_width, arg.length()) - arg.length();

            if (field_width > 0) { // left padded
                for (uint i = 0; i < pad_chars; ++i)
                    *rc++ = fillChar;
            }

            if (locale_arg) {
                memcpy(rc, larg.data(), larg.length()*sizeof(QChar));
                rc += larg.length();
            }
            else {
                memcpy(rc, arg.data(), arg.length()*sizeof(QChar));
                rc += arg.length();
            }

            if (field_width < 0) { // right padded
                for (uint i = 0; i < pad_chars; ++i)
                    *rc++ = fillChar;
            }

            if (++repl_cnt == d.occurrences) {
                memcpy(rc, c, (uc_end - c)*sizeof(QChar));
                rc += uc_end - c;
                Q_ASSERT(rc - result_buff == result_len);
                c = uc_end;
            }
        }
    }
    Q_ASSERT(rc == result_buff + result_len);

    return result;
}

/*!
  Returns a copy of this string with the lowest numbered place marker
  replaced by string \a a, i.e., \c %1, \c %2, ..., \c %99.

  \a fieldWidth specifies the minimum amount of space that argument \a
  a shall occupy. If \a a requires less space than \a fieldWidth, it
  is padded to \a fieldWidth with character \a fillChar.  A positive
  \a fieldWidth produces right-aligned text. A negative \a fieldWidth
  produces left-aligned text.

  This example shows how we might create a \c status string for
  reporting progress while processing a list of files:

  \snippet qstring/main.cpp 11

  First, \c arg(i) replaces \c %1. Then \c arg(total) replaces \c
  %2. Finally, \c arg(fileName) replaces \c %3.

  One advantage of using arg() over asprintf() is that the order of the
  numbered place markers can change, if the application's strings are
  translated into other languages, but each arg() will still replace
  the lowest numbered unreplaced place marker, no matter where it
  appears. Also, if place marker \c %i appears more than once in the
  string, the arg() replaces all of them.

  If there is no unreplaced place marker remaining, a warning message
  is output and the result is undefined. Place marker numbers must be
  in the range 1 to 99.
*/
QString QString::arg(const QString &a, int fieldWidth, QChar fillChar) const
{
    return arg(qToStringViewIgnoringNull(a), fieldWidth, fillChar);
}

/*!
    \overload
    \since 5.10

    Returns a copy of this string with the lowest-numbered place-marker
    replaced by string \a a, i.e., \c %1, \c %2, ..., \c %99.

    \a fieldWidth specifies the minimum amount of space that \a a
    shall occupy. If \a a requires less space than \a fieldWidth, it
    is padded to \a fieldWidth with character \a fillChar.  A positive
    \a fieldWidth produces right-aligned text. A negative \a fieldWidth
    produces left-aligned text.

    This example shows how we might create a \c status string for
    reporting progress while processing a list of files:

    \snippet qstring/main.cpp 11-qstringview

    First, \c arg(i) replaces \c %1. Then \c arg(total) replaces \c
    %2. Finally, \c arg(fileName) replaces \c %3.

    One advantage of using arg() over asprintf() is that the order of the
    numbered place markers can change, if the application's strings are
    translated into other languages, but each arg() will still replace
    the lowest-numbered unreplaced place-marker, no matter where it
    appears. Also, if place-marker \c %i appears more than once in the
    string, arg() replaces all of them.

    If there is no unreplaced place-marker remaining, a warning message
    is printed and the result is undefined. Place-marker numbers must be
    in the range 1 to 99.
*/
QString QString::arg(QStringView a, int fieldWidth, QChar fillChar) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (Q_UNLIKELY(d.occurrences == 0)) {
        qWarning("QString::arg: Argument missing: %ls, %ls", qUtf16Printable(*this),
                  qUtf16Printable(a.toString()));
        return *this;
    }
    return replaceArgEscapes(*this, d, fieldWidth, a, a, fillChar);
}

/*!
    \overload
    \since 5.10

    Returns a copy of this string with the lowest-numbered place-marker
    replaced by string \a a, i.e., \c %1, \c %2, ..., \c %99.

    \a fieldWidth specifies the minimum amount of space that \a a
    shall occupy. If \a a requires less space than \a fieldWidth, it
    is padded to \a fieldWidth with character \a fillChar.  A positive
    \a fieldWidth produces right-aligned text. A negative \a fieldWidth
    produces left-aligned text.

    One advantage of using arg() over asprintf() is that the order of the
    numbered place markers can change, if the application's strings are
    translated into other languages, but each arg() will still replace
    the lowest-numbered unreplaced place-marker, no matter where it
    appears. Also, if place-marker \c %i appears more than once in the
    string, arg() replaces all of them.

    If there is no unreplaced place-marker remaining, a warning message
    is printed and the result is undefined. Place-marker numbers must be
    in the range 1 to 99.
*/
QString QString::arg(QLatin1String a, int fieldWidth, QChar fillChar) const
{
    QVarLengthArray<char16_t> utf16(a.size());
    qt_from_latin1(utf16.data(), a.data(), a.size());
    return arg(QStringView(utf16.data(), utf16.size()), fieldWidth, fillChar);
}

/*! \fn QString QString::arg(int a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  The \a a argument is expressed in base \a base, which is 10 by
  default and must be between 2 and 36. For bases other than 10, \a a
  is treated as an unsigned integer.

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The '%' can be followed by an 'L', in which case the sequence is
  replaced with a localized representation of \a a. The conversion
  uses the default locale, set by QLocale::setDefault(). If no default
  locale was specified, the system locale is used. The 'L' flag is
  ignored if \a base is not 10.

  \snippet qstring/main.cpp 12
  \snippet qstring/main.cpp 14

  \sa {Number Formats}
*/

/*! \fn QString QString::arg(uint a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36.

  \sa {Number Formats}
*/

/*! \fn QString QString::arg(long a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a a argument is expressed in the given \a base, which is 10 by
  default and must be between 2 and 36.

  The '%' can be followed by an 'L', in which case the sequence is
  replaced with a localized representation of \a a. The conversion
  uses the default locale. The default locale is determined from the
  system's locale settings at application startup. It can be changed
  using QLocale::setDefault(). The 'L' flag is ignored if \a base is
  not 10.

  \snippet qstring/main.cpp 12
  \snippet qstring/main.cpp 14

  \sa {Number Formats}
*/

/*!
  \fn QString QString::arg(ulong a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a to a string. The base must be between 2 and 36, with 8
  giving octal, 10 decimal, and 16 hexadecimal numbers.

  \sa {Number Formats}
*/

/*!
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  \sa {Number Formats}
*/
QString QString::arg(qlonglong a, int fieldWidth, int base, QChar fillChar) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning() << "QString::arg: Argument missing:" << *this << ',' << a;
        return *this;
    }

    unsigned flags = QLocaleData::NoFlags;
    // ZeroPadded sorts out left-padding when the fill is zero, to the right of sign:
    if (fillChar == QLatin1Char('0'))
        flags = QLocaleData::ZeroPadded;

    QString arg;
    if (d.occurrences > d.locale_occurrences) {
        arg = QLocaleData::c()->longLongToString(a, -1, base, fieldWidth, flags);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= arg.length());
    }

    QString localeArg;
    if (d.locale_occurrences > 0) {
        QLocale locale;
        if (!(locale.numberOptions() & QLocale::OmitGroupSeparator))
            flags |= QLocaleData::GroupDigits;
        localeArg = locale.d->m_data->longLongToString(a, -1, base, fieldWidth, flags);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= localeArg.length());
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, localeArg, fillChar);
}

/*!
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. \a base must be between 2 and 36, with 8
  giving octal, 10 decimal, and 16 hexadecimal numbers.

  \sa {Number Formats}
*/
QString QString::arg(qulonglong a, int fieldWidth, int base, QChar fillChar) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning() << "QString::arg: Argument missing:" << *this << ',' << a;
        return *this;
    }

    unsigned flags = QLocaleData::NoFlags;
    // ZeroPadded sorts out left-padding when the fill is zero, to the right of sign:
    if (fillChar == QLatin1Char('0'))
        flags = QLocaleData::ZeroPadded;

    QString arg;
    if (d.occurrences > d.locale_occurrences) {
        arg = QLocaleData::c()->unsLongLongToString(a, -1, base, fieldWidth, flags);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= arg.length());
    }

    QString localeArg;
    if (d.locale_occurrences > 0) {
        QLocale locale;
        if (!(locale.numberOptions() & QLocale::OmitGroupSeparator))
            flags |= QLocaleData::GroupDigits;
        localeArg = locale.d->m_data->unsLongLongToString(a, -1, base, fieldWidth, flags);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= localeArg.length());
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, localeArg, fillChar);
}

/*!
  \overload arg()

  \fn QString QString::arg(short a, int fieldWidth, int base, QChar fillChar) const

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  \sa {Number Formats}
*/

/*!
  \fn QString QString::arg(ushort a, int fieldWidth, int base, QChar fillChar) const
  \overload arg()

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar. A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  The \a base argument specifies the base to use when converting the
  integer \a a into a string. The base must be between 2 and 36, with
  8 giving octal, 10 decimal, and 16 hexadecimal numbers.

  \sa {Number Formats}
*/

/*!
    \overload arg()
*/
QString QString::arg(QChar a, int fieldWidth, QChar fillChar) const
{
    return arg(QStringView{&a, 1}, fieldWidth, fillChar);
}

/*!
  \overload arg()

  The \a a argument is interpreted as a Latin-1 character.
*/
QString QString::arg(char a, int fieldWidth, QChar fillChar) const
{
    return arg(QLatin1Char(a), fieldWidth, fillChar);
}

/*!
  \overload arg()

  Argument \a a is formatted according to the specified \a format and
  \a precision. See \l{Floating-point Formats} for details.

  \a fieldWidth specifies the minimum amount of space that \a a is
  padded to and filled with the character \a fillChar.  A positive
  value produces right-aligned text; a negative value produces
  left-aligned text.

  \snippet code/src_corelib_text_qstring.cpp 2

  \sa QLocale::toString(), QLocale::FloatingPointPrecisionOption, {Number Formats}
*/
QString QString::arg(double a, int fieldWidth, char format, int precision, QChar fillChar) const
{
    ArgEscapeData d = findArgEscapes(*this);

    if (d.occurrences == 0) {
        qWarning("QString::arg: Argument missing: %s, %g", toLocal8Bit().data(), a);
        return *this;
    }

    unsigned flags = QLocaleData::NoFlags;
    // ZeroPadded sorts out left-padding when the fill is zero, to the right of sign:
    if (fillChar == QLatin1Char('0'))
        flags |= QLocaleData::ZeroPadded;

    if (qIsUpper(format))
        flags |= QLocaleData::CapitalEorX;

    QLocaleData::DoubleForm form = QLocaleData::DFDecimal;
    switch (qToLower(format)) {
    case 'f':
        form = QLocaleData::DFDecimal;
        break;
    case 'e':
        form = QLocaleData::DFExponent;
        break;
    case 'g':
        form = QLocaleData::DFSignificantDigits;
        break;
    default:
#if defined(QT_CHECK_RANGE)
        qWarning("QString::arg: Invalid format char '%c'", format);
#endif
        break;
    }

    QString arg;
    if (d.occurrences > d.locale_occurrences) {
        arg = QLocaleData::c()->doubleToString(a, precision, form, fieldWidth,
                                               flags | QLocaleData::ZeroPadExponent);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= arg.length());
    }

    QString localeArg;
    if (d.locale_occurrences > 0) {
        QLocale locale;

        const QLocale::NumberOptions numberOptions = locale.numberOptions();
        if (!(numberOptions & QLocale::OmitGroupSeparator))
            flags |= QLocaleData::GroupDigits;
        if (!(numberOptions & QLocale::OmitLeadingZeroInExponent))
            flags |= QLocaleData::ZeroPadExponent;
        if (numberOptions & QLocale::IncludeTrailingZeroesAfterDot)
            flags |= QLocaleData::AddTrailingZeroes;
        localeArg = locale.d->m_data->doubleToString(a, precision, form, fieldWidth, flags);
        Q_ASSERT(fillChar != QLatin1Char('0') || !qIsFinite(a)
                 || fieldWidth <= localeArg.length());
    }

    return replaceArgEscapes(*this, d, fieldWidth, arg, localeArg, fillChar);
}

static inline char16_t to_unicode(const QChar c) { return c.unicode(); }
static inline char16_t to_unicode(const char c) { return QLatin1Char{c}.unicode(); }

template <typename Char>
static int getEscape(const Char *uc, qsizetype *pos, qsizetype len, int maxNumber = 999)
{
    qsizetype i = *pos;
    ++i;
    if (i < len && uc[i] == QLatin1Char('L'))
        ++i;
    if (i < len) {
        int escape = to_unicode(uc[i]) - '0';
        if (uint(escape) >= 10U)
            return -1;
        ++i;
        while (i < len) {
            int digit = to_unicode(uc[i]) - '0';
            if (uint(digit) >= 10U)
                break;
            escape = (escape * 10) + digit;
            ++i;
        }
        if (escape <= maxNumber) {
            *pos = i;
            return escape;
        }
    }
    return -1;
}

/*
    Algorithm for multiArg:

    1. Parse the string as a sequence of verbatim text and placeholders (%L?\d{,3}).
       The L is parsed and accepted for compatibility with non-multi-arg, but since
       multiArg only accepts strings as replacements, the localization request can
       be safely ignored.
    2. The result of step (1) is a list of (string-ref,int)-tuples. The string-ref
       either points at text to be copied verbatim (in which case the int is -1),
       or, initially, at the textual representation of the placeholder. In that case,
       the int contains the numerical number as parsed from the placeholder.
    3. Next, collect all the non-negative ints found, sort them in ascending order and
       remove duplicates.
       3a. If the result has more entries than multiArg() was given replacement strings,
           we have found placeholders we can't satisfy with replacement strings. That is
           fine (there could be another .arg() call coming after this one), so just
           truncate the result to the number of actual multiArg() replacement strings.
       3b. If the result has less entries than multiArg() was given replacement strings,
           the string is missing placeholders. This is an error that the user should be
           warned about.
    4. The result of step (3) is a mapping from the index of any replacement string to
       placeholder number. This is the wrong way around, but since placeholder
       numbers could get as large as 999, while we typically don't have more than 9
       replacement strings, we trade 4K of sparsely-used memory for doing a reverse lookup
       each time we need to map a placeholder number to a replacement string index
       (that's a linear search; but still *much* faster than using an associative container).
    5. Next, for each of the tuples found in step (1), do the following:
       5a. If the int is negative, do nothing.
       5b. Otherwise, if the int is found in the result of step (3) at index I, replace
           the string-ref with a string-ref for the (complete) I'th replacement string.
       5c. Otherwise, do nothing.
    6. Concatenate all string refs into a single result string.
*/

namespace {
struct Part
{
    Part() = default; // for QVarLengthArray; do not use
    constexpr Part(QStringView s, int num = -1)
        : tag{QtPrivate::ArgBase::U16}, number{num}, data{s.utf16()}, size{s.size()} {}
    constexpr Part(QLatin1String s, int num = -1)
        : tag{QtPrivate::ArgBase::L1}, number{num}, data{s.data()}, size{s.size()} {}

    void reset(QStringView s) noexcept { *this = {s, number}; }
    void reset(QLatin1String s) noexcept { *this = {s, number}; }

    QtPrivate::ArgBase::Tag tag;
    int number;
    const void *data;
    qsizetype size;
};
} // unnamed namespace

Q_DECLARE_TYPEINFO(Part, Q_PRIMITIVE_TYPE);

namespace {

enum { ExpectedParts = 32 };

typedef QVarLengthArray<Part, ExpectedParts> ParseResult;
typedef QVarLengthArray<int, ExpectedParts/2> ArgIndexToPlaceholderMap;

template <typename StringView>
static ParseResult parseMultiArgFormatString(StringView s)
{
    ParseResult result;

    const auto uc = s.data();
    const auto len = s.size();
    const auto end = len - 1;
    qsizetype i = 0;
    qsizetype last = 0;

    while (i < end) {
        if (uc[i] == QLatin1Char('%')) {
            qsizetype percent = i;
            int number = getEscape(uc, &i, len);
            if (number != -1) {
                if (last != percent)
                    result.push_back(Part{s.mid(last, percent - last)}); // literal text (incl. failed placeholders)
                result.push_back(Part{s.mid(percent, i - percent), number});  // parsed placeholder
                last = i;
                continue;
            }
        }
        ++i;
    }

    if (last < len)
        result.push_back(Part{s.mid(last, len - last)}); // trailing literal text

    return result;
}

static ArgIndexToPlaceholderMap makeArgIndexToPlaceholderMap(const ParseResult &parts)
{
    ArgIndexToPlaceholderMap result;

    for (Part part : parts) {
        if (part.number >= 0)
            result.push_back(part.number);
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()),
                 result.end());

    return result;
}

static qsizetype resolveStringRefsAndReturnTotalSize(ParseResult &parts, const ArgIndexToPlaceholderMap &argIndexToPlaceholderMap, const QtPrivate::ArgBase *args[])
{
    using namespace QtPrivate;
    qsizetype totalSize = 0;
    for (Part &part : parts) {
        if (part.number != -1) {
            const auto it = std::find(argIndexToPlaceholderMap.begin(), argIndexToPlaceholderMap.end(), part.number);
            if (it != argIndexToPlaceholderMap.end()) {
                const auto &arg = *args[it - argIndexToPlaceholderMap.begin()];
                switch (arg.tag) {
                case ArgBase::L1:
                    part.reset(static_cast<const QLatin1StringArg&>(arg).string);
                    break;
                case ArgBase::U8:
                    Q_UNREACHABLE(); // waiting for QUtf8String...
                    break;
                case ArgBase::U16:
                    part.reset(static_cast<const QStringViewArg&>(arg).string);
                    break;
                }
            }
        }
        totalSize += part.size;
    }
    return totalSize;
}

} // unnamed namespace

Q_ALWAYS_INLINE QString to_string(QLatin1String s) noexcept { return s; }
Q_ALWAYS_INLINE QString to_string(QStringView s) noexcept { return s.toString(); }

template <typename StringView>
static QString argToQStringImpl(StringView pattern, size_t numArgs, const QtPrivate::ArgBase **args)
{
    // Step 1-2 above
    ParseResult parts = parseMultiArgFormatString(pattern);

    // 3-4
    ArgIndexToPlaceholderMap argIndexToPlaceholderMap = makeArgIndexToPlaceholderMap(parts);

    if (static_cast<size_t>(argIndexToPlaceholderMap.size()) > numArgs) // 3a
        argIndexToPlaceholderMap.resize(qsizetype(numArgs));
    else if (Q_UNLIKELY(static_cast<size_t>(argIndexToPlaceholderMap.size()) < numArgs)) // 3b
        qWarning("QString::arg: %d argument(s) missing in %ls",
                 int(numArgs - argIndexToPlaceholderMap.size()), qUtf16Printable(to_string(pattern)));

    // 5
    const qsizetype totalSize = resolveStringRefsAndReturnTotalSize(parts, argIndexToPlaceholderMap, args);

    // 6:
    QString result(totalSize, Qt::Uninitialized);
    auto out = const_cast<QChar*>(result.constData());

    for (Part part : parts) {
        switch (part.tag) {
        case QtPrivate::ArgBase::L1:
            if (part.size) {
                qt_from_latin1(reinterpret_cast<char16_t*>(out),
                               reinterpret_cast<const char*>(part.data), part.size);
            }
            break;
        case QtPrivate::ArgBase::U8:
            Q_UNREACHABLE(); // waiting for QUtf8String
            break;
        case QtPrivate::ArgBase::U16:
            if (part.size)
                memcpy(out, part.data, part.size * sizeof(QChar));
            break;
        }
        out += part.size;
    }

    return result;
}

QString QtPrivate::argToQString(QStringView pattern, size_t n, const ArgBase **args)
{
    return argToQStringImpl(pattern, n, args);
}

QString QtPrivate::argToQString(QLatin1String pattern, size_t n, const ArgBase **args)
{
    return argToQStringImpl(pattern, n, args);
}

/*! \fn bool QString::isSimpleText() const

    \internal
*/
bool QString::isSimpleText() const
{
    const char16_t *p = d.data();
    const char16_t * const end = p + d.size;
    while (p < end) {
        char16_t uc = *p;
        // sort out regions of complex text formatting
        if (uc > 0x058f && (uc < 0x1100 || uc > 0xfb0f)) {
            return false;
        }
        p++;
    }

    return true;
}

/*! \fn bool QString::isRightToLeft() const

    Returns \c true if the string is read right to left.

    \sa QStringView::isRightToLeft()
*/
bool QString::isRightToLeft() const
{
    return QtPrivate::isRightToLeft(QStringView(*this));
}

/*!
    \fn bool QString::isValidUtf16() const noexcept
    \since 5.15

    Returns \c true if the string contains valid UTF-16 encoded data,
    or \c false otherwise.

    Note that this function does not perform any special validation of the
    data; it merely checks if it can be successfully decoded from UTF-16.
    The data is assumed to be in host byte order; the presence of a BOM
    is meaningless.

    \sa QStringView::isValidUtf16()
*/

/*! \fn QChar *QString::data()

    Returns a pointer to the data stored in the QString. The pointer
    can be used to access and modify the characters that compose the
    string.

    Unlike constData() and unicode(), the returned data is always
    '\\0'-terminated.

    Example:

    \snippet qstring/main.cpp 19

    Note that the pointer remains valid only as long as the string is
    not modified by other means. For read-only access, constData() is
    faster because it never causes a \l{deep copy} to occur.

    \sa constData(), operator[]()
*/

/*! \fn const QChar *QString::data() const

    \overload

    \note The returned string may not be '\\0'-terminated.
    Use size() to determine the length of the array.

    \sa fromRawData()
*/

/*! \fn const QChar *QString::constData() const

    Returns a pointer to the data stored in the QString. The pointer
    can be used to access the characters that compose the string.

    Note that the pointer remains valid only as long as the string is
    not modified.

    \note The returned string may not be '\\0'-terminated.
    Use size() to determine the length of the array.

    \sa data(), operator[](), fromRawData()
*/

/*! \fn void QString::push_front(const QString &other)

    This function is provided for STL compatibility, prepending the
    given \a other string to the beginning of this string. It is
    equivalent to \c prepend(other).

    \sa prepend()
*/

/*! \fn void QString::push_front(QChar ch)

    \overload

    Prepends the given \a ch character to the beginning of this string.
*/

/*! \fn void QString::push_back(const QString &other)

    This function is provided for STL compatibility, appending the
    given \a other string onto the end of this string. It is
    equivalent to \c append(other).

    \sa append()
*/

/*! \fn void QString::push_back(QChar ch)

    \overload

    Appends the given \a ch character onto the end of this string.
*/

/*!
    \since 6.1

    Removes from the string the characters in the half-open range
    [ \a first , \a last ). Returns an iterator to the character
    referred to by \a last before the erase.
*/
QString::iterator QString::erase(QString::const_iterator first, QString::const_iterator last)
{
    const auto start = std::distance(cbegin(), first);
    const auto len = std::distance(first, last);
    remove(start, len);
    return begin() + start;
}

/*! \fn void QString::shrink_to_fit()
    \since 5.10

    This function is provided for STL compatibility. It is
    equivalent to squeeze().

    \sa squeeze()
*/

/*!
    \fn std::string QString::toStdString() const

    Returns a std::string object with the data contained in this
    QString. The Unicode data is converted into 8-bit characters using
    the toUtf8() function.

    This method is mostly useful to pass a QString to a function
    that accepts a std::string object.

    \sa toLatin1(), toUtf8(), toLocal8Bit(), QByteArray::toStdString()
*/

/*!
    Constructs a QString that uses the first \a size Unicode characters
    in the array \a unicode. The data in \a unicode is \e not
    copied. The caller must be able to guarantee that \a unicode will
    not be deleted or modified as long as the QString (or an
    unmodified copy of it) exists.

    Any attempts to modify the QString or copies of it will cause it
    to create a deep copy of the data, ensuring that the raw data
    isn't modified.

    Here is an example of how we can use a QRegularExpression on raw data in
    memory without requiring to copy the data into a QString:

    \snippet qstring/main.cpp 22
    \snippet qstring/main.cpp 23

    \warning A string created with fromRawData() is \e not
    '\\0'-terminated, unless the raw data contains a '\\0' character
    at position \a size. This means unicode() will \e not return a
    '\\0'-terminated string (although utf16() does, at the cost of
    copying the raw data).

    \sa fromUtf16(), setRawData()
*/
QString QString::fromRawData(const QChar *unicode, qsizetype size)
{
    return QString(DataPointer::fromRawData(const_cast<char16_t *>(reinterpret_cast<const char16_t *>(unicode)), size));
}

/*!
    \since 4.7

    Resets the QString to use the first \a size Unicode characters
    in the array \a unicode. The data in \a unicode is \e not
    copied. The caller must be able to guarantee that \a unicode will
    not be deleted or modified as long as the QString (or an
    unmodified copy of it) exists.

    This function can be used instead of fromRawData() to re-use
    existings QString objects to save memory re-allocations.

    \sa fromRawData()
*/
QString &QString::setRawData(const QChar *unicode, qsizetype size)
{
    if (!unicode || !size) {
        clear();
    }
    *this = fromRawData(unicode, size);
    return *this;
}

/*! \fn QString QString::fromStdU16String(const std::u16string &str)
    \since 5.5

    Returns a copy of the \a str string. The given string is assumed
    to be encoded in UTF-16.

    \sa fromUtf16(), fromStdWString(), fromStdU32String()
*/

/*!
    \fn std::u16string QString::toStdU16String() const
    \since 5.5

    Returns a std::u16string object with the data contained in this
    QString. The Unicode data is the same as returned by the utf16()
    method.

    \sa utf16(), toStdWString(), toStdU32String()
*/

/*! \fn QString QString::fromStdU32String(const std::u32string &str)
    \since 5.5

    Returns a copy of the \a str string. The given string is assumed
    to be encoded in UCS-4.

    \sa fromUcs4(), fromStdWString(), fromStdU16String()
*/

/*!
    \fn std::u32string QString::toStdU32String() const
    \since 5.5

    Returns a std::u32string object with the data contained in this
    QString. The Unicode data is the same as returned by the toUcs4()
    method.

    \sa toUcs4(), toStdWString(), toStdU16String()
*/

/*! \class QLatin1String
    \inmodule QtCore
    \brief The QLatin1String class provides a thin wrapper around an US-ASCII/Latin-1 encoded string literal.

    \ingroup string-processing
    \reentrant

    Many of QString's member functions are overloaded to accept
    \c{const char *} instead of QString. This includes the copy
    constructor, the assignment operator, the comparison operators,
    and various other functions such as \l{QString::insert()}{insert()},
    \l{QString::append()}{append()}, and \l{QString::prepend()}{prepend()}.
    Some of these functions are optimized to avoid constructing a
    QString object for the \c{const char *} data. For example,
    assuming \c str is a QString,

    \snippet code/src_corelib_text_qstring.cpp 3

    is much faster than

    \snippet code/src_corelib_text_qstring.cpp 4

    because it doesn't construct four temporary QString objects and
    make a deep copy of the character data.

    However, that is not true for all QString member functions that take
    \c{const char *} and therefore applications should assume a temporary will
    be created, such as in

    \snippet code/src_corelib_text_qstring.cpp 4bis

    Applications that define \l QT_NO_CAST_FROM_ASCII (as explained
    in the QString documentation) don't have access to QString's
    \c{const char *} API. To provide an efficient way of specifying
    constant Latin-1 strings, Qt provides the QLatin1String, which is
    just a very thin wrapper around a \c{const char *}. Using
    QLatin1String, the example code above becomes

    \snippet code/src_corelib_text_qstring.cpp 5

    This is a bit longer to type, but it provides exactly the same
    benefits as the first version of the code, and is faster than
    converting the Latin-1 strings using QString::fromLatin1().

    Thanks to the QString(QLatin1String) constructor,
    QLatin1String can be used everywhere a QString is expected. For
    example:

    \snippet code/src_corelib_text_qstring.cpp 6

    \note If the function you're calling with a QLatin1String
    argument isn't actually overloaded to take QLatin1String, the
    implicit conversion to QString will trigger a memory allocation,
    which is usually what you want to avoid by using QLatin1String
    in the first place. In those cases, using QStringLiteral may be
    the better option.

    \sa QString, QLatin1Char, {QStringLiteral()}{QStringLiteral},
    QT_NO_CAST_FROM_ASCII
*/

/*!
    \typedef QLatin1String::value_type
    \since 5.10

    Alias for \c{const char}. Provided for compatibility with the STL.
*/

/*!
    \typedef QLatin1String::difference_type
    \since 5.10

    Alias for \c{qsizetype}. Provided for compatibility with the STL.
*/

/*!
    \typedef QLatin1String::size_type
    \since 5.10

    Alias for \c{qsizetype}. Provided for compatibility with the STL.

    \note In version prior to Qt 6, this was an alias for \c{int},
    restricting the amount of data that could be held in a QLatin1String
    on 64-bit architectures.
*/

/*!
    \typedef QLatin1String::reference
    \since 5.10

    Alias for \c{value_type &}. Provided for compatibility with the STL.
*/

/*!
    \typedef QLatin1String::const_reference
    \since 5.11

    Alias for \c{reference}. Provided for compatibility with the STL.
*/

/*!
    \typedef QLatin1String::iterator
    \since 5.10

    QLatin1String does not support mutable iterators, so this is the same
    as const_iterator.

    \sa const_iterator, reverse_iterator
*/

/*!
    \typedef QLatin1String::const_iterator
    \since 5.10

    \sa iterator, const_reverse_iterator
*/

/*!
    \typedef QLatin1String::reverse_iterator
    \since 5.10

    QLatin1String does not support mutable reverse iterators, so this is the
    same as const_reverse_iterator.

    \sa const_reverse_iterator, iterator
*/

/*!
    \typedef QLatin1String::const_reverse_iterator
    \since 5.10

    \sa reverse_iterator, const_iterator
*/

/*! \fn QLatin1String::QLatin1String()
    \since 5.6

    Constructs a QLatin1String object that stores a nullptr.
*/

/*! \fn QLatin1String::QLatin1String(const char *str)

    Constructs a QLatin1String object that stores \a str.

    The string data is \e not copied. The caller must be able to
    guarantee that \a str will not be deleted or modified as long as
    the QLatin1String object exists.

    \sa latin1()
*/

/*! \fn QLatin1String::QLatin1String(const char *str, qsizetype size)

    Constructs a QLatin1String object that stores \a str with \a size.

    The string data is \e not copied. The caller must be able to
    guarantee that \a str will not be deleted or modified as long as
    the QLatin1String object exists.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, which will be converted to Unicode null characters (U+0000) if this
    string is used by QString. This behavior is different from Qt 5.x.

    \sa latin1()
*/

/*!
    \fn QLatin1String::QLatin1String(const char *first, const char *last)
    \since 5.10

    Constructs a QLatin1String object that stores \a first with length
    (\a last - \a first).

    The range \c{[first,last)} must remain valid for the lifetime of
    this Latin-1 string object.

    Passing \nullptr as \a first is safe if \a last is \nullptr,
    too, and results in a null Latin-1 string.

    The behavior is undefined if \a last precedes \a first, \a first
    is \nullptr and \a last is not, or if \c{last - first >
    INT_MAX}.
*/

/*! \fn QLatin1String::QLatin1String(const QByteArray &str)

    Constructs a QLatin1String object that stores \a str.

    The string data is \e not copied. The caller must be able to
    guarantee that \a str will not be deleted or modified as long as
    the QLatin1String object exists.

    \sa latin1()
*/

/*! \fn QLatin1String::QLatin1String(QByteArrayView str)
    \since 6.3

    Constructs a QLatin1String object that stores \a str.

    The string data is \e not copied. The caller must be able to
    guarantee that the data which \a str is pointing to will not
    be deleted or modified as long as the QLatin1String object
    exists. The size is obtained from \a str as-is, without checking
    for a null-terminator.

    \note: any null ('\\0') bytes in the byte array will be included in this
    string, which will be converted to Unicode null characters (U+0000) if this
    string is used by QString.

    \sa latin1()
*/

/*!
    \fn QString QLatin1String::toString() const
    \since 6.0

    Converts this Latin-1 string into a QString. Equivalent to
    \code
    return QString(*this);
    \endcode
*/

/*! \fn const char *QLatin1String::latin1() const

    Returns the Latin-1 string stored in this object.
*/

/*! \fn const char *QLatin1String::data() const

    Returns the Latin-1 string stored in this object.
*/

/*! \fn qsizetype QLatin1String::size() const

    Returns the size of the Latin-1 string stored in this object.

    \note In version prior to Qt 6, this function returned \c{int},
    restricting the amount of data that could be held in a QLatin1String
    on 64-bit architectures.
*/

/*! \fn bool QLatin1String::isNull() const
    \since 5.10

    Returns whether the Latin-1 string stored in this object is null
    (\c{data() == nullptr}) or not.

    \sa isEmpty(), data()
*/

/*! \fn bool QLatin1String::isEmpty() const
    \since 5.10

    Returns whether the Latin-1 string stored in this object is empty
    (\c{size() == 0}) or not.

    \sa isNull(), size()
*/

/*! \fn QLatin1Char QLatin1String::at(qsizetype pos) const
    \since 5.8

    Returns the character at position \a pos in this object.

    \note This function performs no error checking.
    The behavior is undefined when \a pos < 0 or \a pos >= size().

    \sa operator[]()
*/

/*! \fn QLatin1Char QLatin1String::operator[](qsizetype pos) const
    \since 5.8

    Returns the character at position \a pos in this object.

    \note This function performs no error checking.
    The behavior is undefined when \a pos < 0 or \a pos >= size().

    \sa at()
*/

/*!
    \fn QLatin1Char QLatin1String::front() const
    \since 5.10

    Returns the first character in the string.
    Same as \c{at(0)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa back(), at(), operator[]()
*/

/*!
    \fn QLatin1Char QLatin1String::back() const
    \since 5.10

    Returns the last character in the string.
    Same as \c{at(size() - 1)}.

    This function is provided for STL compatibility.

    \warning Calling this function on an empty string constitutes
    undefined behavior.

    \sa front(), at(), operator[]()
*/

/*!
    \fn int QLatin1String::compare(QStringView str, Qt::CaseSensitivity cs) const
    \fn int QLatin1String::compare(QLatin1String l1, Qt::CaseSensitivity cs) const
    \fn int QLatin1String::compare(QChar ch) const
    \fn int QLatin1String::compare(QChar ch, Qt::CaseSensitivity cs) const
    \since 5.14

    Returns an integer that compares to zero as this Latin-1 string compares to the
    string-view \a str, Latin-1 string \a l1, or character \a ch, respectively.

    If \a cs is Qt::CaseSensitive (the default), the comparison is case sensitive;
    otherwise the comparison is case-insensitive.

    \sa operator==(), operator<(), operator>()
*/


/*!
    \fn bool QLatin1String::startsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \fn bool QLatin1String::startsWith(QLatin1String l1, Qt::CaseSensitivity cs) const
    \since 5.10
    \fn bool QLatin1String::startsWith(QChar ch) const
    \since 5.10
    \fn bool QLatin1String::startsWith(QChar ch, Qt::CaseSensitivity cs) const
    \since 5.10

    Returns \c true if this Latin-1 string starts with string-view \a str,
    Latin-1 string \a l1, or character \a ch, respectively;
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (the default), the search is case-sensitive;
    otherwise the search is case-insensitive.

    \sa endsWith()
*/

/*!
    \fn bool QLatin1String::endsWith(QStringView str, Qt::CaseSensitivity cs) const
    \since 5.10
    \fn bool QLatin1String::endsWith(QLatin1String l1, Qt::CaseSensitivity cs) const
    \since 5.10
    \fn bool QLatin1String::endsWith(QChar ch) const
    \since 5.10
    \fn bool QLatin1String::endsWith(QChar ch, Qt::CaseSensitivity cs) const
    \since 5.10

    Returns \c true if this Latin-1 string ends with string-view \a str,
    Latin-1 string \a l1, or character \a ch, respectively;
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (the default), the search is case-sensitive;
    otherwise the search is case-insensitive.

    \sa startsWith()
*/

/*!
    \fn qsizetype QLatin1String::indexOf(QStringView str, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \fn qsizetype QLatin1String::indexOf(QLatin1String l1, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \fn qsizetype QLatin1String::indexOf(QChar c, qsizetype from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 5.14

    Returns the index position of the first occurrence of the string-view
    \a str, Latin-1 string \a l1, or character \a ch, respectively, in this
    Latin-1 string, searching forward from index position \a from.
    Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    If \a from is -1, the search starts at the last character; if it is
    -2, at the next to last character and so on.

    \sa QString::indexOf()
*/

/*!
    \fn bool QLatin1String::contains(QStringView str, Qt::CaseSensitivity cs) const
    \fn bool QLatin1String::contains(QLatin1String l1, Qt::CaseSensitivity cs) const
    \fn bool QLatin1String::contains(QChar c, Qt::CaseSensitivity cs) const
    \since 5.14

    Returns \c true if this Latin-1 string contains an occurrence of the
    string-view \a str, Latin-1 string \a l1, or character \a ch;
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (the default), the search is
    case-sensitive; otherwise the search is case-insensitive.

    \sa indexOf(), QStringView::contains(), QStringView::indexOf(),
    QString::indexOf()
*/

/*!
    \fn qsizetype QLatin1String::lastIndexOf(QStringView str, qsizetype from, Qt::CaseSensitivity cs) const
    \fn qsizetype QLatin1String::lastIndexOf(QLatin1String l1, qsizetype from, Qt::CaseSensitivity cs) const
    \fn qsizetype QLatin1String::lastIndexOf(QChar c, qsizetype from, Qt::CaseSensitivity cs) const
    \since 5.14

    Returns the index position of the last occurrence of the string-view \a str,
    Latin-1 string \a l1, or character \a ch, respectively, in this Latin-1
    string, searching backward from index position \a from.
    Returns -1 if \a str is not found.

    If \a from is -1, the search starts at the last character;
    if \a from is -2, at the next to last character and so on.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.

    \note When searching for a 0-length \a str or \a l1, the match at
    the end of the data is excluded from the search by a negative \a
    from, even though \c{-1} is normally thought of as searching from
    the end of the string: the match at the end is \e after the last
    character, so it is excluded. To include such a final empty match,
    either give a positive value for \a from or omit the \a from
    parameter entirely.

    \sa indexOf(), QStringView::lastIndexOf(), QStringView::indexOf(),
    QString::indexOf()
*/

/*!
    \fn qsizetype QLatin1String::lastIndexOf(QStringView str, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \fn qsizetype QLatin1String::lastIndexOf(QLatin1String l1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
    \since 6.2
    \overload lastIndexOf()

    Returns the index position of the last occurrence of the
    string-view \a str or Latin-1 string \a l1, respectively, in this
    Latin-1 string. Returns -1 if \a str is not found.

    If \a cs is Qt::CaseSensitive (default), the search is case
    sensitive; otherwise the search is case insensitive.
*/

/*!
    \fn QLatin1String::const_iterator QLatin1String::begin() const
    \since 5.10

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing to the
    first character in the string.

    This function is provided for STL compatibility.

    \sa end(), cbegin(), rbegin(), data()
*/

/*!
    \fn QLatin1String::const_iterator QLatin1String::cbegin() const
    \since 5.10

    Same as begin().

    This function is provided for STL compatibility.

    \sa cend(), begin(), crbegin(), data()
*/

/*!
    \fn QLatin1String::const_iterator QLatin1String::end() const
    \since 5.10

    Returns a const \l{STL-style iterators}{STL-style iterator} pointing just
    after the last character in the string.

    This function is provided for STL compatibility.

    \sa begin(), cend(), rend()
*/

/*! \fn QLatin1String::const_iterator QLatin1String::cend() const
    \since 5.10

    Same as end().

    This function is provided for STL compatibility.

    \sa cbegin(), end(), crend()
*/

/*!
    \fn QLatin1String::const_reverse_iterator QLatin1String::rbegin() const
    \since 5.10

    Returns a const \l{STL-style iterators}{STL-style} reverse iterator pointing
    to the first character in the string, in reverse order.

    This function is provided for STL compatibility.

    \sa rend(), crbegin(), begin()
*/

/*!
    \fn QLatin1String::const_reverse_iterator QLatin1String::crbegin() const
    \since 5.10

    Same as rbegin().

    This function is provided for STL compatibility.

    \sa crend(), rbegin(), cbegin()
*/

/*!
    \fn QLatin1String::const_reverse_iterator QLatin1String::rend() const
    \since 5.10

    Returns a \l{STL-style iterators}{STL-style} reverse iterator pointing just
    after the last character in the string, in reverse order.

    This function is provided for STL compatibility.

    \sa rbegin(), crend(), end()
*/

/*!
    \fn QLatin1String::const_reverse_iterator QLatin1String::crend() const
    \since 5.10

    Same as rend().

    This function is provided for STL compatibility.

    \sa crbegin(), rend(), cend()
*/

/*!
    \fn QLatin1String QLatin1String::mid(qsizetype start, qsizetype length) const
    \since 5.8

    Returns the substring of length \a length starting at position
    \a start in this Latin-1 string.

    If you know that \a start and \a length cannot be out of bounds, use
    sliced() instead in new code, because it is faster.

    Returns an empty Latin-1 string if \a start exceeds the
    length of this Latin-1 string. If there are less than \a length characters
    available in this Latin-1 string starting at \a start, or if
    \a length is negative (default), the function returns all characters that
    are available from \a start.

    \sa first(), last(), sliced(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::left(qsizetype length) const
    \since 5.8

    If you know that \a length cannot be out of bounds, use first() instead in
    new code, because it is faster.

    Returns the substring of length \a length starting at position
    0 in this Latin-1 string.

    The entire Latin-1 string is returned if \a length is greater than or equal
    to size(), or less than zero.

    \sa first(), last(), sliced(), startsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::right(qsizetype length) const
    \since 5.8

    If you know that \a length cannot be out of bounds, use last() instead in
    new code, because it is faster.

    Returns the substring of length \a length starting at position
    size() - \a length in this Latin-1 string.

    The entire Latin-1 string is returned if \a length is greater than or equal
    to size(), or less than zero.

    \sa first(), last(), sliced(), endsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::first(qsizetype n) const
    \since 6.0

    Returns a Latin-1 string that contains the first \a n characters
    of this Latin-1 string.

    \note The behavior is undefined when \a n < 0 or \a n > size().

    \sa last(), startsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::last(qsizetype n) const
    \since 6.0

    Returns a Latin-1 string that contains the last \a n characters
    of this Latin-1 string.

    \note The behavior is undefined when \a n < 0 or \a n > size().

    \sa first(), endsWith(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::sliced(qsizetype pos, qsizetype n) const
    \since 6.0

    Returns a Latin-1 string that points to \a n characters of this
    Latin-1 string, starting at position \a pos.

    \note The behavior is undefined when \a pos < 0, \a n < 0,
    or \c{pos + n > size()}.

    \sa first(), last(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::sliced(qsizetype pos) const
    \since 6.0

    Returns a Latin-1 string starting at position \a pos in this
    Latin-1 string, and extending to its end.

    \note The behavior is undefined when \a pos < 0 or \a pos > size().

    \sa first(), last(), chopped(), chop(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::chopped(qsizetype length) const
    \since 5.10

    Returns the substring of length size() - \a length starting at the
    beginning of this object.

    Same as \c{left(size() - length)}.

    \note The behavior is undefined when \a length < 0 or \a length > size().

    \sa sliced(), first(), last(), chop(), truncate()
*/

/*!
    \fn void QLatin1String::truncate(qsizetype length)
    \since 5.10

    Truncates this string to length \a length.

    Same as \c{*this = left(length)}.

    \note The behavior is undefined when \a length < 0 or \a length > size().

    \sa sliced(), first(), last(), chopped(), chop()
*/

/*!
    \fn void QLatin1String::chop(qsizetype length)
    \since 5.10

    Truncates this string by \a length characters.

    Same as \c{*this = left(size() - length)}.

    \note The behavior is undefined when \a length < 0 or \a length > size().

    \sa sliced(), first(), last(), chopped(), truncate()
*/

/*!
    \fn QLatin1String QLatin1String::trimmed() const
    \since 5.10

    Strips leading and trailing whitespace and returns the result.

    Whitespace means any character for which QChar::isSpace() returns
    \c true. This includes the ASCII characters '\\t', '\\n', '\\v',
    '\\f', '\\r', and ' '.
*/

/*!
    \fn bool QLatin1String::operator==(const char *other) const
    \since 4.3

    Returns \c true if the string is equal to const char pointer \a other;
    otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator==(const QByteArray &other) const
    \since 5.0
    \overload

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \fn bool QLatin1String::operator!=(const char *other) const
    \since 4.3

    Returns \c true if this string is not equal to const char pointer \a other;
    otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator!=(const QByteArray &other) const
    \since 5.0
    \overload operator!=()

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \fn bool QLatin1String::operator>(const char *other) const
    \since 4.3

    Returns \c true if this string is lexically greater than const char pointer
    \a other; otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator>(const QByteArray &other) const
    \since 5.0
    \overload

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining \l QT_NO_CAST_FROM_ASCII
    when you compile your applications. This can be useful if you want
    to ensure that all user-visible strings go through QObject::tr(),
    for example.
*/

/*!
    \fn bool QLatin1String::operator<(const char *other) const
    \since 4.3

    Returns \c true if this string is lexically less than const char pointer
    \a other; otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator<(const QByteArray &other) const
    \since 5.0
    \overload

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \fn bool QLatin1String::operator>=(const char *other) const
    \since 4.3

    Returns \c true if this string is lexically greater than or equal to
    const char pointer \a other; otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator>=(const QByteArray &other) const
    \since 5.0
    \overload

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*!
    \fn bool QLatin1String::operator<=(const char *other) const
    \since 4.3

    Returns \c true if this string is lexically less than or equal to
    const char pointer \a other; otherwise returns \c false.

    The \a other const char pointer is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.

    \sa {Comparing Strings}
*/

/*!
    \fn bool QLatin1String::operator<=(const QByteArray &other) const
    \since 5.0
    \overload

    The \a other byte array is converted to a QString using
    the QString::fromUtf8() function.

    You can disable this operator by defining
    \l QT_NO_CAST_FROM_ASCII when you compile your applications. This
    can be useful if you want to ensure that all user-visible strings
    go through QObject::tr(), for example.
*/

/*! \fn bool QLatin1String::operator==(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically equal to string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically not equal to string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically less than string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically less than or equal to
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically greater than string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(QLatin1String s1, QLatin1String s2)

    Returns \c true if string \a s1 is lexically greater than or equal
    to string \a s2; otherwise returns \c false.
*/

/*! \fn bool QLatin1String::operator==(QChar ch, QLatin1String s)

    Returns \c true if char \a ch is lexically equal to string \a s;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(QChar ch, QLatin1String s)

    Returns \c true if char \a ch is lexically less than string \a s;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(QChar ch, QLatin1String s)
    Returns \c true if char \a ch is lexically greater than string \a s;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(QChar ch, QLatin1String s)

    Returns \c true if char \a ch is lexically not equal to string \a s;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(QChar ch, QLatin1String s)

    Returns \c true if char \a ch is lexically less than or equal to
    string \a s; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(QChar ch, QLatin1String s)

    Returns \c true if char \a ch is lexically greater than or equal to
    string \a s; otherwise returns \c false.
*/

/*! \fn bool QLatin1String::operator==(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically equal to char \a ch;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically less than char \a ch;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically greater than char \a ch;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically not equal to char \a ch;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically less than or equal to
    char \a ch; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(QLatin1String s, QChar ch)

    Returns \c true if string \a s is lexically greater than or equal to
    char \a ch; otherwise returns \c false.
*/

/*! \fn bool QLatin1String::operator==(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically equal to string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically less than string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically greater than string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically not equal to string \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically less than or equal to
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(QStringView s1, QLatin1String s2)

    Returns \c true if string view \a s1 is lexically greater than or equal to
    string \a s2; otherwise returns \c false.
*/

/*! \fn bool QLatin1String::operator==(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically equal to string view \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically less than string view \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically greater than string view \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically not equal to string view \a s2;
    otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically less than or equal to
    string view \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(QLatin1String s1, QStringView s2)

    Returns \c true if string \a s1 is lexically greater than or equal to
    string view \a s2; otherwise returns \c false.
*/

/*! \fn bool QLatin1String::operator==(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically equal to
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically less than
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically greater than
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator!=(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically not equal to
    string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator<=(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically less than or
    equal to string \a s2; otherwise returns \c false.
*/
/*! \fn bool QLatin1String::operator>=(const char *s1, QLatin1String s2)

    Returns \c true if const char pointer \a s1 is lexically greater than or
    equal to string \a s2; otherwise returns \c false.
*/

#if !defined(QT_NO_DATASTREAM) || defined(QT_BOOTSTRAPPED)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QString &string)
    \relates QString

    Writes the given \a string to the specified \a stream.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator<<(QDataStream &out, const QString &str)
{
    if (out.version() == 1) {
        out << str.toLatin1();
    } else {
        if (!str.isNull() || out.version() < 3) {
            if ((out.byteOrder() == QDataStream::BigEndian) == (QSysInfo::ByteOrder == QSysInfo::BigEndian)) {
                out.writeBytes(reinterpret_cast<const char *>(str.unicode()),
                               static_cast<uint>(sizeof(QChar) * str.length()));
            } else {
                QVarLengthArray<char16_t> buffer(str.length());
                qbswap<sizeof(char16_t)>(str.constData(), str.length(), buffer.data());
                out.writeBytes(reinterpret_cast<const char *>(buffer.data()),
                               static_cast<uint>(sizeof(char16_t) * buffer.size()));
            }
        } else {
            // write null marker
            out << (quint32)0xffffffff;
        }
    }
    return out;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QString &string)
    \relates QString

    Reads a string from the specified \a stream into the given \a string.

    \sa {Serializing Qt Data Types}
*/

QDataStream &operator>>(QDataStream &in, QString &str)
{
    if (in.version() == 1) {
        QByteArray l;
        in >> l;
        str = QString::fromLatin1(l);
    } else {
        quint32 bytes = 0;
        in >> bytes;                                  // read size of string
        if (bytes == 0xffffffff) {                    // null string
            str.clear();
        } else if (bytes > 0) {                       // not empty
            if (bytes & 0x1) {
                str.clear();
                in.setStatus(QDataStream::ReadCorruptData);
                return in;
            }

            const quint32 Step = 1024 * 1024;
            quint32 len = bytes / 2;
            quint32 allocated = 0;

            while (allocated < len) {
                int blockSize = qMin(Step, len - allocated);
                str.resize(allocated + blockSize);
                if (in.readRawData(reinterpret_cast<char *>(str.data()) + allocated * 2,
                                   blockSize * 2) != blockSize * 2) {
                    str.clear();
                    in.setStatus(QDataStream::ReadPastEnd);
                    return in;
                }
                allocated += blockSize;
            }

            if ((in.byteOrder() == QDataStream::BigEndian)
                    != (QSysInfo::ByteOrder == QSysInfo::BigEndian)) {
                char16_t *data = reinterpret_cast<char16_t *>(str.data());
                qbswap<sizeof(*data)>(data, len, data);
            }
        } else {
            str = QString(QLatin1String(""));
        }
    }
    return in;
}
#endif // QT_NO_DATASTREAM

/*!
    \typedef QString::Data
    \internal
*/

/*!
    \typedef QString::DataPtr
    \internal
*/

/*!
    \fn DataPtr & QString::data_ptr()
    \internal
*/

/*!
    \since 5.11
    \internal
    \relates QStringView

    Returns \c true if the string is read right to left.

    \sa QString::isRightToLeft()
*/
bool QtPrivate::isRightToLeft(QStringView string) noexcept
{
    const char16_t *p = string.utf16();
    const char16_t * const end = p + string.size();
    int isolateLevel = 0;
    while (p < end) {
        uint ucs4 = *p;
        if (QChar::isHighSurrogate(ucs4) && p < end - 1) {
            char16_t low = p[1];
            if (QChar::isLowSurrogate(low)) {
                ucs4 = QChar::surrogateToUcs4(ucs4, low);
                ++p;
            }
        }
        switch (QChar::direction(ucs4))
        {
        case QChar::DirRLI:
        case QChar::DirLRI:
        case QChar::DirFSI:
            ++isolateLevel;
            break;
        case QChar::DirPDI:
            if (isolateLevel)
                --isolateLevel;
            break;
        case QChar::DirL:
            if (isolateLevel)
                break;
            return false;
        case QChar::DirR:
        case QChar::DirAL:
            if (isolateLevel)
                break;
            return true;
        default:
            break;
        }
        ++p;
    }
    return false;
}

qsizetype QtPrivate::count(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    qsizetype num = 0;
    qsizetype i = -1;
    if (haystack.size() > 500 && needle.size() > 5) {
        QStringMatcher matcher(needle, cs);
        while ((i = matcher.indexIn(haystack, i + 1)) != -1)
            ++num;
    } else {
        while ((i = QtPrivate::findString(haystack, i + 1, needle, cs)) != -1)
            ++num;
    }
    return num;
}

qsizetype QtPrivate::count(QStringView haystack, QChar ch, Qt::CaseSensitivity cs) noexcept
{
    qsizetype num = 0;
    if (cs == Qt::CaseSensitive) {
        for (QChar c : haystack) {
            if (c == ch)
                ++num;
        }
    } else {
        ch = foldCase(ch);
        for (QChar c : haystack) {
            if (foldCase(c) == ch)
                ++num;
        }
    }
    return num;
}

template <typename Haystack, typename Needle>
bool qt_starts_with_impl(Haystack haystack, Needle needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.isNull())
        return needle.isNull();
    const auto haystackLen = haystack.size();
    const auto needleLen = needle.size();
    if (haystackLen == 0)
        return needleLen == 0;
    if (needleLen > haystackLen)
        return false;

    return QtPrivate::compareStrings(haystack.left(needleLen), needle, cs) == 0;
}

static inline bool qt_starts_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
{
    return qt_starts_with_impl(haystack, needle, cs);
}

static inline bool qt_starts_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
{
    return qt_starts_with_impl(haystack, needle, cs);
}

static inline bool qt_starts_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs)
{
    return haystack.size()
           && (cs == Qt::CaseSensitive ? haystack.front() == needle
                                       : foldCase(haystack.front()) == foldCase(needle));
}

/*!
    \fn bool QtPrivate::startsWith(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::startsWith(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::startsWith(QLatin1String haystack, QStringView needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::startsWith(QLatin1String haystack, QLatin1String needle, Qt::CaseSensitivity cs)
    \since 5.10
    \internal
    \relates QStringView

    Returns \c true if \a haystack starts with \a needle,
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (the default), the search is case-sensitive;
    otherwise the search is case-insensitive.

    \sa QtPrivate::endsWith(), QString::endsWith(), QStringView::endsWith(), QLatin1String::endsWith()
*/

bool QtPrivate::startsWith(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_starts_with_impl(haystack, needle, cs);
}

bool QtPrivate::startsWith(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_starts_with_impl(haystack, needle, cs);
}

bool QtPrivate::startsWith(QLatin1String haystack, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_starts_with_impl(haystack, needle, cs);
}

bool QtPrivate::startsWith(QLatin1String haystack, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_starts_with_impl(haystack, needle, cs);
}

template <typename Haystack, typename Needle>
bool qt_ends_with_impl(Haystack haystack, Needle needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.isNull())
        return needle.isNull();
    const auto haystackLen = haystack.size();
    const auto needleLen = needle.size();
    if (haystackLen == 0)
        return needleLen == 0;
    if (haystackLen < needleLen)
        return false;

    return QtPrivate::compareStrings(haystack.right(needleLen), needle, cs) == 0;
}

static inline bool qt_ends_with(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
{
    return qt_ends_with_impl(haystack, needle, cs);
}

static inline bool qt_ends_with(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
{
    return qt_ends_with_impl(haystack, needle, cs);
}

static inline bool qt_ends_with(QStringView haystack, QChar needle, Qt::CaseSensitivity cs)
{
    return haystack.size()
           && (cs == Qt::CaseSensitive ? haystack.back() == needle
                                       : foldCase(haystack.back()) == foldCase(needle));
}

/*!
    \fn bool QtPrivate::endsWith(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::endsWith(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::endsWith(QLatin1String haystack, QStringView needle, Qt::CaseSensitivity cs)
    \since 5.10
    \fn bool QtPrivate::endsWith(QLatin1String haystack, QLatin1String needle, Qt::CaseSensitivity cs)
    \since 5.10
    \internal
    \relates QStringView

    Returns \c true if \a haystack ends with \a needle,
    otherwise returns \c false.

    If \a cs is Qt::CaseSensitive (the default), the search is case-sensitive;
    otherwise the search is case-insensitive.

    \sa QtPrivate::startsWith(), QString::endsWith(), QStringView::endsWith(), QLatin1String::endsWith()
*/

bool QtPrivate::endsWith(QStringView haystack, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_ends_with_impl(haystack, needle, cs);
}

bool QtPrivate::endsWith(QStringView haystack, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_ends_with_impl(haystack, needle, cs);
}

bool QtPrivate::endsWith(QLatin1String haystack, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_ends_with_impl(haystack, needle, cs);
}

bool QtPrivate::endsWith(QLatin1String haystack, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qt_ends_with_impl(haystack, needle, cs);
}

namespace {
template <typename Pointer>
char32_t foldCaseHelper(Pointer ch, Pointer start) = delete;

template <>
char32_t foldCaseHelper<const QChar*>(const QChar* ch, const QChar* start)
{
    return foldCase(reinterpret_cast<const char16_t*>(ch),
                    reinterpret_cast<const char16_t*>(start));
}

template <>
char32_t foldCaseHelper<const char*>(const char* ch, const char*)
{
    return foldCase(char16_t(uchar(*ch)));
}

template <typename T>
char16_t valueTypeToUtf16(T t) = delete;

template <>
char16_t valueTypeToUtf16<QChar>(QChar t)
{
    return t.unicode();
}

template <>
char16_t valueTypeToUtf16<char>(char t)
{
    return char16_t{uchar(t)};
}
}

/*!
    \internal

    Returns the index position of the first occurrence of the
    character \a ch in the string given by \a str and \a len,
    searching forward from index
    position \a from. Returns -1 if \a ch could not be found.
*/

static inline qsizetype qFindChar(QStringView str, QChar ch, qsizetype from, Qt::CaseSensitivity cs) noexcept
{
    if (from < 0)
        from = qMax(from + str.size(), qsizetype(0));
    if (from < str.size()) {
        const char16_t *s = str.utf16();
        char16_t c = ch.unicode();
        const char16_t *n = s + from;
        const char16_t *e = s + str.size();
        if (cs == Qt::CaseSensitive) {
            n = QtPrivate::qustrchr(QStringView(n, e), c);
            if (n != e)
                return n - s;
        } else {
            c = foldCase(c);
            --n;
            while (++n != e)
                if (foldCase(*n) == c)
                    return n - s;
        }
    }
    return -1;
}

qsizetype QtPrivate::findString(QStringView haystack0, qsizetype from, QStringView needle0, Qt::CaseSensitivity cs) noexcept
{
    const qsizetype l = haystack0.size();
    const qsizetype sl = needle0.size();
    if (from < 0)
        from += l;
    if (std::size_t(sl + from) > std::size_t(l))
        return -1;
    if (!sl)
        return from;
    if (!l)
        return -1;

    if (sl == 1)
        return qFindChar(haystack0, needle0[0], from, cs);

    /*
        We use the Boyer-Moore algorithm in cases where the overhead
        for the skip table should pay off, otherwise we use a simple
        hash function.
    */
    if (l > 500 && sl > 5)
        return qFindStringBoyerMoore(haystack0, from, needle0, cs);

    auto sv = [sl](const char16_t *v) { return QStringView(v, sl); };
    /*
        We use some hashing for efficiency's sake. Instead of
        comparing strings, we compare the hash value of str with that
        of a part of this QString. Only if that matches, we call
        qt_string_compare().
    */
    const char16_t *needle = needle0.utf16();
    const char16_t *haystack = haystack0.utf16() + from;
    const char16_t *end = haystack0.utf16() + (l - sl);
    const std::size_t sl_minus_1 = sl - 1;
    std::size_t hashNeedle = 0, hashHaystack = 0;
    qsizetype idx;

    if (cs == Qt::CaseSensitive) {
        for (idx = 0; idx < sl; ++idx) {
            hashNeedle = ((hashNeedle<<1) + needle[idx]);
            hashHaystack = ((hashHaystack<<1) + haystack[idx]);
        }
        hashHaystack -= haystack[sl_minus_1];

        while (haystack <= end) {
            hashHaystack += haystack[sl_minus_1];
            if (hashHaystack == hashNeedle
                 && QtPrivate::compareStrings(needle0, sv(haystack), Qt::CaseSensitive) == 0)
                return haystack - haystack0.utf16();

            REHASH(*haystack);
            ++haystack;
        }
    } else {
        const char16_t *haystack_start = haystack0.utf16();
        for (idx = 0; idx < sl; ++idx) {
            hashNeedle = (hashNeedle<<1) + foldCase(needle + idx, needle);
            hashHaystack = (hashHaystack<<1) + foldCase(haystack + idx, haystack_start);
        }
        hashHaystack -= foldCase(haystack + sl_minus_1, haystack_start);

        while (haystack <= end) {
            hashHaystack += foldCase(haystack + sl_minus_1, haystack_start);
            if (hashHaystack == hashNeedle
                 && QtPrivate::compareStrings(needle0, sv(haystack), Qt::CaseInsensitive) == 0)
                return haystack - haystack0.utf16();

            REHASH(foldCase(haystack, haystack_start));
            ++haystack;
        }
    }
    return -1;
}

template <typename Haystack>
static inline qsizetype qLastIndexOf(Haystack haystack, QChar needle,
                                     qsizetype from, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.size() == 0)
        return -1;
    if (from < 0)
        from += haystack.size();
    else if (std::size_t(from) > std::size_t(haystack.size()))
        from = haystack.size() - 1;
    if (from >= 0) {
        char16_t c = needle.unicode();
        const auto b = haystack.data();
        auto n = b + from;
        if (cs == Qt::CaseSensitive) {
            for (; n >= b; --n)
                if (valueTypeToUtf16(*n) == c)
                    return n - b;
        } else {
            c = foldCase(c);
            for (; n >= b; --n)
                if (foldCase(valueTypeToUtf16(*n)) == c)
                    return n - b;
        }
    }
    return -1;
}

template<typename Haystack, typename Needle>
static qsizetype qLastIndexOf(Haystack haystack0, qsizetype from,
                              Needle needle0, Qt::CaseSensitivity cs) noexcept
{
    const qsizetype sl = needle0.size();
    if (sl == 1)
        return qLastIndexOf(haystack0, needle0.front(), from, cs);

    const qsizetype l = haystack0.size();
    if (from < 0)
        from += l;
    if (from == l && sl == 0)
        return from;
    const qsizetype delta = l - sl;
    if (std::size_t(from) > std::size_t(l) || delta < 0)
        return -1;
    if (from > delta)
        from = delta;

    auto sv = [sl](const typename Haystack::value_type *v) { return Haystack(v, sl); };

    auto haystack = haystack0.data();
    const auto needle = needle0.data();
    const auto *end = haystack;
    haystack += from;
    const std::size_t sl_minus_1 = sl ? sl - 1 : 0;
    const auto *n = needle + sl_minus_1;
    const auto *h = haystack + sl_minus_1;
    std::size_t hashNeedle = 0, hashHaystack = 0;
    qsizetype idx;

    if (cs == Qt::CaseSensitive) {
        for (idx = 0; idx < sl; ++idx) {
            hashNeedle = (hashNeedle << 1) + valueTypeToUtf16(*(n - idx));
            hashHaystack = (hashHaystack << 1) + valueTypeToUtf16(*(h - idx));
        }
        hashHaystack -= valueTypeToUtf16(*haystack);

        while (haystack >= end) {
            hashHaystack += valueTypeToUtf16(*haystack);
            if (hashHaystack == hashNeedle
                 && QtPrivate::compareStrings(needle0, sv(haystack), Qt::CaseSensitive) == 0)
                return haystack - end;
            --haystack;
            REHASH(valueTypeToUtf16(haystack[sl]));
        }
    } else {
        for (idx = 0; idx < sl; ++idx) {
            hashNeedle = (hashNeedle << 1) + foldCaseHelper(n - idx, needle);
            hashHaystack = (hashHaystack << 1) + foldCaseHelper(h - idx, end);
        }
        hashHaystack -= foldCaseHelper(haystack, end);

        while (haystack >= end) {
            hashHaystack += foldCaseHelper(haystack, end);
            if (hashHaystack == hashNeedle
                 && QtPrivate::compareStrings(sv(haystack), needle0, Qt::CaseInsensitive) == 0)
                return haystack - end;
            --haystack;
            REHASH(foldCaseHelper(haystack + sl, end));
        }
    }
    return -1;
}

qsizetype QtPrivate::findString(QStringView haystack, qsizetype from, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.size() < needle.size())
        return -1;

    QVarLengthArray<char16_t> s(needle.size());
    qt_from_latin1(s.data(), needle.latin1(), needle.size());
    return QtPrivate::findString(haystack, from, QStringView(reinterpret_cast<const QChar*>(s.constData()), s.size()), cs);
}

qsizetype QtPrivate::findString(QLatin1String haystack, qsizetype from, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.size() < needle.size())
        return -1;

    QVarLengthArray<char16_t> s(haystack.size());
    qt_from_latin1(s.data(), haystack.latin1(), haystack.size());
    return QtPrivate::findString(QStringView(reinterpret_cast<const QChar*>(s.constData()), s.size()), from, needle, cs);
}

qsizetype QtPrivate::findString(QLatin1String haystack, qsizetype from, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    if (haystack.size() < needle.size())
        return -1;

    QVarLengthArray<char16_t> h(haystack.size());
    qt_from_latin1(h.data(), haystack.latin1(), haystack.size());
    QVarLengthArray<char16_t> n(needle.size());
    qt_from_latin1(n.data(), needle.latin1(), needle.size());
    return QtPrivate::findString(QStringView(reinterpret_cast<const QChar*>(h.constData()), h.size()), from,
                                 QStringView(reinterpret_cast<const QChar*>(n.constData()), n.size()), cs);
}

qsizetype QtPrivate::lastIndexOf(QStringView haystack, qsizetype from, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qLastIndexOf(haystack, from, needle, cs);
}

qsizetype QtPrivate::lastIndexOf(QStringView haystack, qsizetype from, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qLastIndexOf(haystack, from, needle, cs);
}

qsizetype QtPrivate::lastIndexOf(QLatin1String haystack, qsizetype from, QStringView needle, Qt::CaseSensitivity cs) noexcept
{
    return qLastIndexOf(haystack, from, needle, cs);
}

qsizetype QtPrivate::lastIndexOf(QLatin1String haystack, qsizetype from, QLatin1String needle, Qt::CaseSensitivity cs) noexcept
{
    return qLastIndexOf(haystack, from, needle, cs);
}

#if QT_CONFIG(regularexpression)
qsizetype QtPrivate::indexOf(QStringView haystack, const QRegularExpression &re, qsizetype from, QRegularExpressionMatch *rmatch)
{
    if (!re.isValid()) {
        qWarning("QStringView::indexOf: invalid QRegularExpression object");
        return -1;
    }

    QRegularExpressionMatch match = re.match(haystack, from);
    if (match.hasMatch()) {
        const qsizetype ret = match.capturedStart();
        if (rmatch)
            *rmatch = std::move(match);
        return ret;
    }

    return -1;
}

qsizetype QtPrivate::lastIndexOf(QStringView haystack, const QRegularExpression &re, qsizetype from, QRegularExpressionMatch *rmatch)
{
    if (!re.isValid()) {
        qWarning("QStringView::lastIndexOf: invalid QRegularExpression object");
        return -1;
    }

    qsizetype endpos = (from < 0) ? (haystack.size() + from + 1) : (from + 1);
    QRegularExpressionMatchIterator iterator = re.globalMatch(haystack);
    qsizetype lastIndex = -1;
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        qsizetype start = match.capturedStart();
        if (start < endpos) {
            lastIndex = start;
            if (rmatch)
                *rmatch = std::move(match);
        } else {
            break;
        }
    }

    return lastIndex;
}

bool QtPrivate::contains(QStringView haystack, const QRegularExpression &re, QRegularExpressionMatch *rmatch)
{
    if (!re.isValid()) {
        qWarning("QStringView::contains: invalid QRegularExpression object");
        return false;
    }
    QRegularExpressionMatch m = re.match(haystack);
    bool hasMatch = m.hasMatch();
    if (hasMatch && rmatch)
        *rmatch = std::move(m);
    return hasMatch;
}

qsizetype QtPrivate::count(QStringView haystack, const QRegularExpression &re)
{
    if (!re.isValid()) {
        qWarning("QStringView::count: invalid QRegularExpression object");
        return 0;
    }
    qsizetype count = 0;
    qsizetype index = -1;
    qsizetype len = haystack.length();
    while (index <= len - 1) {
        QRegularExpressionMatch match = re.match(haystack, index + 1);
        if (!match.hasMatch())
            break;
        index = match.capturedStart();
        count++;
    }
    return count;
}

#endif // QT_CONFIG(regularexpression)

/*!
    \since 5.0

    Converts a plain text string to an HTML string with
    HTML metacharacters \c{<}, \c{>}, \c{&}, and \c{"} replaced by HTML
    entities.

    Example:

    \snippet code/src_corelib_text_qstring.cpp 7
*/
QString QString::toHtmlEscaped() const
{
    QString rich;
    const qsizetype len = length();
    rich.reserve(qsizetype(len * 1.1));
    for (QChar ch : *this) {
        if (ch == u'<')
            rich += QLatin1String("&lt;");
        else if (ch == u'>')
            rich += QLatin1String("&gt;");
        else if (ch == u'&')
            rich += QLatin1String("&amp;");
        else if (ch == u'"')
            rich += QLatin1String("&quot;");
        else
            rich += ch;
    }
    rich.squeeze();
    return rich;
}

/*!
  \macro QStringLiteral(str)
  \relates QString

  The macro generates the data for a QString out of the string literal \a str
  at compile time. Creating a QString from it is free in this case, and the
  generated string data is stored in the read-only segment of the compiled
  object file.

  If you have code that looks like this:

  \snippet code/src_corelib_text_qstring.cpp 9

  then a temporary QString will be created to be passed as the \c{hasAttribute}
  function parameter. This can be quite expensive, as it involves a memory
  allocation and the copy/conversion of the data into QString's internal
  encoding.

  This cost can be avoided by using QStringLiteral instead:

  \snippet code/src_corelib_text_qstring.cpp 10

  In this case, QString's internal data will be generated at compile time; no
  conversion or allocation will occur at runtime.

  Using QStringLiteral instead of a double quoted plain C++ string literal can
  significantly speed up creation of QString instances from data known at
  compile time.

  \note QLatin1String can still be more efficient than QStringLiteral
  when the string is passed to a function that has an overload taking
  QLatin1String and this overload avoids conversion to QString.  For
  instance, QString::operator==() can compare to a QLatin1String
  directly:

  \snippet code/src_corelib_text_qstring.cpp 11

  \note Some compilers have bugs encoding strings containing characters outside
  the US-ASCII character set. Make sure you prefix your string with \c{u} in
  those cases. It is optional otherwise.

  \sa QByteArrayLiteral
*/

/*!
  \fn QtLiterals::operator""_qs(const char16_t *str, size_t size)

  \relates QString
  \since 6.2

  Literal operator that creates a QString out of the first \a size characters in
  the char16_t string literal \a str.

  The QString is created at compile time, and the generated string data is stored
  in the read-only segment of the compiled object file. Duplicate literals may
  share the same read-only memory. This functionality is interchangeable with
  QStringLiteral, but saves typing when many string literals are present in the
  code.

  The following code creates a QString:
  \code
  auto str = u"hello"_qs;
  \endcode

  \sa QStringLiteral, QtLiterals::operator""_qba(const char *str, size_t size)
*/

/*!
    \internal
 */
void QAbstractConcatenable::appendLatin1To(QLatin1String in, QChar *out) noexcept
{
    qt_from_latin1(reinterpret_cast<char16_t *>(out), in.data(), size_t(in.size()));
}

double QStringView::toDouble(bool *ok) const
{
    return QLocaleData::c()->stringToDouble(*this, ok, QLocale::RejectGroupSeparator);
}

float QStringView::toFloat(bool *ok) const
{
    return QLocaleData::convertDoubleToFloat(toDouble(ok), ok);
}

/*!
  \fn template <typename T> qsizetype erase(QString &s, const T &t)
  \relates QString
  \since 6.1

  Removes all elements that compare equal to \a t from the
  string \a s. Returns the number of elements removed, if any.

  \sa erase_if
*/

/*!
  \fn template <typename Predicate> qsizetype erase_if(QString &s, Predicate pred)
  \relates QString
  \since 6.1

  Removes all elements for which the predicate \a pred returns true
  from the string \a s. Returns the number of elements removed, if
  any.

  \sa erase
*/

QT_END_NAMESPACE
