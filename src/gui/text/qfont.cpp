/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qfont.h"
#include "qdebug.h"
#include "qpaintdevice.h"
#include "qfontdatabase.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qpainter.h"
#include "qhash.h"
#include "qdatastream.h"
#include "qguiapplication.h"
#include "qstringlist.h"
#include "qscreen.h"

#include "qthread.h"
#include "qthreadstorage.h"

#include "qfont_p.h"
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <limits.h>

#include <qpa/qplatformscreen.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformfontdatabase.h>
#include <QtGui/private/qguiapplication_p.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

// #define QFONTCACHE_DEBUG
#ifdef QFONTCACHE_DEBUG
#  define FC_DEBUG qDebug
#else
#  define FC_DEBUG if (false) qDebug
#endif

QT_BEGIN_NAMESPACE

#ifndef QFONTCACHE_DECREASE_TRIGGER_LIMIT
#  define QFONTCACHE_DECREASE_TRIGGER_LIMIT 256
#endif

bool QFontDef::exactMatch(const QFontDef &other) const
{
    /*
      QFontDef comparison is more complicated than just simple
      per-member comparisons.

      When comparing point/pixel sizes, either point or pixelsize
      could be -1.  in This case we have to compare the non negative
      size value.

      This test will fail if the point-sizes differ by 1/2 point or
      more or they do not round to the same value.  We have to do this
      since our API still uses 'int' point-sizes in the API, but store
      deci-point-sizes internally.

      To compare the family members, we need to parse the font names
      and compare the family/foundry strings separately.  This allows
      us to compare e.g. "Helvetica" and "Helvetica [Adobe]" with
      positive results.
    */
    if (pixelSize != -1 && other.pixelSize != -1) {
        if (pixelSize != other.pixelSize)
            return false;
    } else if (pointSize != -1 && other.pointSize != -1) {
        if (pointSize != other.pointSize)
            return false;
    } else {
        return false;
    }

    if (!ignorePitch && !other.ignorePitch && fixedPitch != other.fixedPitch)
        return false;

    if (stretch != 0 && other.stretch != 0 && stretch != other.stretch)
        return false;

    QString this_family, this_foundry, other_family, other_foundry;
    for (int i = 0; i < families.size(); ++i) {
        QFontDatabasePrivate::parseFontName(families.at(i), this_foundry, this_family);
        QFontDatabasePrivate::parseFontName(other.families.at(i), other_foundry, other_family);
        if (this_family != other_family || this_foundry != other_foundry)
            return false;
    }

    return (styleHint     == other.styleHint
            && styleStrategy == other.styleStrategy
            && weight        == other.weight
            && style        == other.style
            && this_family   == other_family
            && (styleName.isEmpty() || other.styleName.isEmpty() || styleName == other.styleName)
            && (this_foundry.isEmpty()
                || other_foundry.isEmpty()
                || this_foundry == other_foundry)
       );
}

extern bool qt_is_gui_used;

Q_GUI_EXPORT int qt_defaultDpiX()
{
    if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi))
        return 96;

    if (!qt_is_gui_used)
        return 75;

    if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchX());

    //PI has not been initialised, or it is being initialised. Give a default dpi
    return 100;
}

Q_GUI_EXPORT int qt_defaultDpiY()
{
    if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi))
        return 96;

    if (!qt_is_gui_used)
        return 75;

    if (const QScreen *screen = QGuiApplication::primaryScreen())
        return qRound(screen->logicalDotsPerInchY());

    //PI has not been initialised, or it is being initialised. Give a default dpi
    return 100;
}

Q_GUI_EXPORT int qt_defaultDpi()
{
    return qt_defaultDpiY();
}

/* Helper function to convert between legacy Qt and OpenType font weights. */
static int convertWeights(int weight, bool inverted)
{
    static const QVarLengthArray<QPair<int, int>, 9> legacyToOpenTypeMap = {
        { 0, QFont::Thin },    { 12, QFont::ExtraLight }, { 25, QFont::Light },
        { 50, QFont::Normal }, { 57, QFont::Medium },     { 63, QFont::DemiBold },
        { 75, QFont::Bold },   { 81, QFont::ExtraBold },  { 87, QFont::Black },
    };

    int closestDist = INT_MAX;
    int result = -1;

    // Go through and find the closest mapped value
    for (auto mapping : legacyToOpenTypeMap) {
        const int weightOld = inverted ? mapping.second : mapping.first;
        const int weightNew = inverted ? mapping.first : mapping.second;
        const int dist = qAbs(weightOld - weight);
        if (dist < closestDist) {
            result = weightNew;
            closestDist = dist;
        } else {
            // Break early since following values will be further away
            break;
        }
    }

    return result;
}

// Splits the family string on a comma and returns the list based on that
static QStringList splitIntoFamilies(const QString &family)
{
    QStringList familyList;
    if (family.isEmpty())
        return familyList;
    const auto list = QStringView{family}.split(QLatin1Char(','));
    const int numFamilies = list.size();
    familyList.reserve(numFamilies);
    for (int i = 0; i < numFamilies; ++i) {
        auto str = list.at(i).trimmed();
        if ((str.startsWith(QLatin1Char('"')) && str.endsWith(QLatin1Char('"')))
            || (str.startsWith(QLatin1Char('\'')) && str.endsWith(QLatin1Char('\'')))) {
            str = str.mid(1, str.length() - 2);
        }
        familyList << str.toString();
    }
    return familyList;
}

/* Converts from legacy Qt font weight (Qt < 6.0) to OpenType font weight (Qt >= 6.0) */
Q_GUI_EXPORT int qt_legacyToOpenTypeWeight(int weight)
{
    return convertWeights(weight, false);
}

/* Converts from  OpenType font weight (Qt >= 6.0) to legacy Qt font weight (Qt < 6.0) */
Q_GUI_EXPORT int qt_openTypeToLegacyWeight(int weight)
{
    return convertWeights(weight, true);
}

QFontPrivate::QFontPrivate()
    : engineData(nullptr), dpi(qt_defaultDpi()),
      underline(false), overline(false), strikeOut(false), kerning(true),
      capital(0), letterSpacingIsAbsolute(false), scFont(nullptr)
{
}

QFontPrivate::QFontPrivate(const QFontPrivate &other)
    : request(other.request), engineData(nullptr), dpi(other.dpi),
      underline(other.underline), overline(other.overline),
      strikeOut(other.strikeOut), kerning(other.kerning),
      capital(other.capital), letterSpacingIsAbsolute(other.letterSpacingIsAbsolute),
      letterSpacing(other.letterSpacing), wordSpacing(other.wordSpacing),
      scFont(other.scFont)
{
    if (scFont && scFont != this)
        scFont->ref.ref();
}

QFontPrivate::~QFontPrivate()
{
    if (engineData && !engineData->ref.deref())
        delete engineData;
    engineData = nullptr;
    if (scFont && scFont != this) {
        if (!scFont->ref.deref())
            delete scFont;
    }
    scFont = nullptr;
}

extern QRecursiveMutex *qt_fontdatabase_mutex();

#define QT_FONT_ENGINE_FROM_DATA(data, script) data->engines[script]

QFontEngine *QFontPrivate::engineForScript(int script) const
{
    QMutexLocker locker(qt_fontdatabase_mutex());
    if (script <= QChar::Script_Latin)
        script = QChar::Script_Common;
    if (engineData && engineData->fontCacheId != QFontCache::instance()->id()) {
        // throw out engineData that came from a different thread
        if (!engineData->ref.deref())
            delete engineData;
        engineData = nullptr;
    }
    if (!engineData || !QT_FONT_ENGINE_FROM_DATA(engineData, script))
        QFontDatabasePrivate::load(this, script);
    return QT_FONT_ENGINE_FROM_DATA(engineData, script);
}

void QFontPrivate::alterCharForCapitalization(QChar &c) const {
    switch (capital) {
    case QFont::AllUppercase:
    case QFont::SmallCaps:
        c = c.toUpper();
        break;
    case QFont::AllLowercase:
        c = c.toLower();
        break;
    case QFont::MixedCase:
        break;
    }
}

QFontPrivate *QFontPrivate::smallCapsFontPrivate() const
{
    if (scFont)
        return scFont;
    QFont font(const_cast<QFontPrivate *>(this));
    qreal pointSize = font.pointSizeF();
    if (pointSize > 0)
        font.setPointSizeF(pointSize * .7);
    else
        font.setPixelSize((font.pixelSize() * 7 + 5) / 10);
    scFont = font.d.data();
    if (scFont != this)
        scFont->ref.ref();
    return scFont;
}


void QFontPrivate::resolve(uint mask, const QFontPrivate *other)
{
    Q_ASSERT(other != nullptr);

    dpi = other->dpi;

    if ((mask & QFont::AllPropertiesResolved) == QFont::AllPropertiesResolved) return;

    // assign the unset-bits with the set-bits of the other font def
    if (!(mask & QFont::FamiliesResolved))
        request.families = other->request.families;

    if (! (mask & QFont::StyleNameResolved))
        request.styleName = other->request.styleName;

    if (! (mask & QFont::SizeResolved)) {
        request.pointSize = other->request.pointSize;
        request.pixelSize = other->request.pixelSize;
    }

    if (! (mask & QFont::StyleHintResolved))
        request.styleHint = other->request.styleHint;

    if (! (mask & QFont::StyleStrategyResolved))
        request.styleStrategy = other->request.styleStrategy;

    if (! (mask & QFont::WeightResolved))
        request.weight = other->request.weight;

    if (! (mask & QFont::StyleResolved))
        request.style = other->request.style;

    if (! (mask & QFont::FixedPitchResolved))
        request.fixedPitch = other->request.fixedPitch;

    if (! (mask & QFont::StretchResolved))
        request.stretch = other->request.stretch;

    if (! (mask & QFont::HintingPreferenceResolved))
        request.hintingPreference = other->request.hintingPreference;

    if (! (mask & QFont::UnderlineResolved))
        underline = other->underline;

    if (! (mask & QFont::OverlineResolved))
        overline = other->overline;

    if (! (mask & QFont::StrikeOutResolved))
        strikeOut = other->strikeOut;

    if (! (mask & QFont::KerningResolved))
        kerning = other->kerning;

    if (! (mask & QFont::LetterSpacingResolved)) {
        letterSpacing = other->letterSpacing;
        letterSpacingIsAbsolute = other->letterSpacingIsAbsolute;
    }
    if (! (mask & QFont::WordSpacingResolved))
        wordSpacing = other->wordSpacing;
    if (! (mask & QFont::CapitalizationResolved))
        capital = other->capital;
}




QFontEngineData::QFontEngineData()
    : ref(0), fontCacheId(QFontCache::instance()->id())
{
    memset(engines, 0, QChar::ScriptCount * sizeof(QFontEngine *));
}

QFontEngineData::~QFontEngineData()
{
    Q_ASSERT(ref.loadRelaxed() == 0);
    for (int i = 0; i < QChar::ScriptCount; ++i) {
        if (engines[i]) {
            if (!engines[i]->ref.deref())
                delete engines[i];
            engines[i] = nullptr;
        }
    }
}




/*!
    \class QFont
    \reentrant

    \brief The QFont class specifies a query for a font used for drawing text.

    \ingroup painting
    \ingroup appearance
    \ingroup shared
    \ingroup richtext-processing
    \inmodule QtGui

    QFont can be regarded as a query for one or more fonts on the system.

    When you create a QFont object you specify various attributes that
    you want the font to have. Qt will use the font with the specified
    attributes, or if no matching font exists, Qt will use the closest
    matching installed font. The attributes of the font that is
    actually used are retrievable from a QFontInfo object. If the
    window system provides an exact match exactMatch() returns \c true.
    Use QFontMetricsF to get measurements, e.g. the pixel length of a
    string using QFontMetrics::width().

    Attributes which are not specifically set will not affect the font
    selection algorithm, and default values will be preferred instead.

    To load a specific physical font, typically represented by a single file,
    use QRawFont instead.

    Note that a QGuiApplication instance must exist before a QFont can be
    used. You can set the application's default font with
    QGuiApplication::setFont().

    If a chosen font does not include all the characters that
    need to be displayed, QFont will try to find the characters in the
    nearest equivalent fonts. When a QPainter draws a character from a
    font the QFont will report whether or not it has the character; if
    it does not, QPainter will draw an unfilled square.

    Create QFonts like this:

    \snippet code/src_gui_text_qfont.cpp 0

    The attributes set in the constructor can also be set later, e.g.
    setFamily(), setPointSize(), setPointSizeF(), setWeight() and
    setItalic(). The remaining attributes must be set after
    construction, e.g. setBold(), setUnderline(), setOverline(),
    setStrikeOut() and setFixedPitch(). QFontInfo objects should be
    created \e after the font's attributes have been set. A QFontInfo
    object will not change, even if you change the font's
    attributes. The corresponding "get" functions, e.g. family(),
    pointSize(), etc., return the values that were set, even though
    the values used may differ. The actual values are available from a
    QFontInfo object.

    If the requested font family is unavailable you can influence the
    \l{#fontmatching}{font matching algorithm} by choosing a
    particular \l{QFont::StyleHint} and \l{QFont::StyleStrategy} with
    setStyleHint(). The default family (corresponding to the current
    style hint) is returned by defaultFamily().

    You can provide substitutions for font family names using
    insertSubstitution() and insertSubstitutions(). Substitutions can
    be removed with removeSubstitutions(). Use substitute() to retrieve
    a family's first substitute, or the family name itself if it has
    no substitutes. Use substitutes() to retrieve a list of a family's
    substitutes (which may be empty). After substituting a font, you must
    trigger the updating of the font by destroying and re-creating all
    QFont objects.

    Every QFont has a key() which you can use, for example, as the key
    in a cache or dictionary. If you want to store a user's font
    preferences you could use QSettings, writing the font information
    with toString() and reading it back with fromString(). The
    operator<<() and operator>>() functions are also available, but
    they work on a data stream.

    It is possible to set the height of characters shown on the screen
    to a specified number of pixels with setPixelSize(); however using
    setPointSize() has a similar effect and provides device
    independence.

    Loading fonts can be expensive, especially on X11. QFont contains
    extensive optimizations to make the copying of QFont objects fast,
    and to cache the results of the slow window system functions it
    depends upon.

    \target fontmatching
    The font matching algorithm works as follows:
    \list 1
    \li The specified font families (set by setFamilies()) are searched for.
    \li If not, a replacement font that supports the writing system is
        selected. The font matching algorithm will try to find the
        best match for all the properties set in the QFont. How this is
        done varies from platform to platform.
    \li If no font exists on the system that can support the text,
        then special "missing character" boxes will be shown in its place.
    \endlist

    \note If the selected font, though supporting the writing system in general,
    is missing glyphs for one or more specific characters, then Qt will try to
    find a fallback font for this or these particular characters. This feature
    can be disabled using QFont::NoFontMerging style strategy.

    In Windows a request for the "Courier" font is automatically changed to
    "Courier New", an improved version of Courier that allows for smooth scaling.
    The older "Courier" bitmap font can be selected by setting the PreferBitmap
    style strategy (see setStyleStrategy()).

    Once a font is found, the remaining attributes are matched in order of
    priority:
    \list 1
    \li fixedPitch()
    \li pointSize() (see below)
    \li weight()
    \li style()
    \endlist

    If you have a font which matches on family, even if none of the
    other attributes match, this font will be chosen in preference to
    a font which doesn't match on family but which does match on the
    other attributes. This is because font family is the dominant
    search criteria.

    The point size is defined to match if it is within 20% of the
    requested point size. When several fonts match and are only
    distinguished by point size, the font with the closest point size
    to the one requested will be chosen.

    The actual family, font size, weight and other font attributes
    used for drawing text will depend on what's available for the
    chosen family under the window system. A QFontInfo object can be
    used to determine the actual values used for drawing the text.

    Examples:

    \snippet code/src_gui_text_qfont.cpp 1
    If you had both an Adobe and a Cronyx Helvetica, you might get
    either.

    \snippet code/src_gui_text_qfont.cpp 2

    You can specify the foundry you want in the family name. The font f
    in the above example will be set to "Helvetica
    [Cronyx]".

    To determine the attributes of the font actually used in the window
    system, use a QFontInfo object, e.g.

    \snippet code/src_gui_text_qfont.cpp 3

    To find out font metrics use a QFontMetrics object, e.g.

    \snippet code/src_gui_text_qfont.cpp 4

    For more general information on fonts, see the
    \l{comp.fonts FAQ}{comp.fonts FAQ}.
    Information on encodings can be found from the
    \l{UTR17} page.

    \sa QFontMetrics, QFontInfo, QFontDatabase, {Character Map Example}
*/

/*!
    \internal
    \enum QFont::ResolveProperties

    This enum describes the properties of a QFont that can be set on a font
    individually and then considered resolved.

    \value FamilyResolved
    \value FamiliesResolved
    \value SizeResolved
    \value StyleHintResolved
    \value StyleStrategyResolved
    \value WeightResolved
    \value StyleResolved
    \value UnderlineResolved
    \value OverlineResolved
    \value StrikeOutResolved
    \value FixedPitchResolved
    \value StretchResolved
    \value KerningResolved
    \value CapitalizationResolved
    \value LetterSpacingResolved
    \value WordSpacingResolved
    \value CompletelyResolved
*/

/*!
    \enum QFont::Style

    This enum describes the different styles of glyphs that are used to
    display text.

    \value StyleNormal  Normal glyphs used in unstyled text.
    \value StyleItalic  Italic glyphs that are specifically designed for
                        the purpose of representing italicized text.
    \value StyleOblique Glyphs with an italic appearance that are typically
                        based on the unstyled glyphs, but are not fine-tuned
                        for the purpose of representing italicized text.

    \sa Weight
*/

/*!
    \fn QFont &QFont::operator=(QFont &&other)

    Move-assigns \a other to this QFont instance.

    \since 5.2
*/

/*!
  \since 5.13
  Constructs a font from \a font for use on the paint device \a pd.
*/
QFont::QFont(const QFont &font, const QPaintDevice *pd)
    : resolve_mask(font.resolve_mask)
{
    Q_ASSERT(pd);
    const int dpi = pd->logicalDpiY();
    if (font.d->dpi != dpi) {
        d = new QFontPrivate(*font.d);
        d->dpi = dpi;
    } else {
        d = font.d;
    }
}

/*!
  \internal
*/
QFont::QFont(QFontPrivate *data)
    : d(data), resolve_mask(QFont::AllPropertiesResolved)
{
}

/*! \internal
    Detaches the font object from common font data.
*/
void QFont::detach()
{
    if (d->ref.loadRelaxed() == 1) {
        if (d->engineData && !d->engineData->ref.deref())
            delete d->engineData;
        d->engineData = nullptr;
        if (d->scFont && d->scFont != d.data()) {
            if (!d->scFont->ref.deref())
                delete d->scFont;
        }
        d->scFont = nullptr;
        return;
    }

    d.detach();
}

/*!
    \internal
    Detaches the font object from common font attributes data.
    Call this instead of QFont::detach() if the only font attributes data
    has been changed (underline, letterSpacing, kerning, etc.).
*/
void QFontPrivate::detachButKeepEngineData(QFont *font)
{
    if (font->d->ref.loadRelaxed() == 1)
        return;

    QFontEngineData *engineData = font->d->engineData;
    if (engineData)
        engineData->ref.ref();
    font->d.detach();
    font->d->engineData = engineData;
}

/*!
    Constructs a font object that uses the application's default font.

    \sa QGuiApplication::setFont(), QGuiApplication::font()
*/
QFont::QFont()
    : d(QGuiApplicationPrivate::instance() ? QGuiApplication::font().d.data() : new QFontPrivate()), resolve_mask(0)
{
}

/*!
    Constructs a font object with the specified \a family, \a
    pointSize, \a weight and \a italic settings.

    If \a pointSize is zero or negative, the point size of the font
    is set to a system-dependent default value. Generally, this is
    12 points.

    The \a family name may optionally also include a foundry name,
    e.g. "Helvetica [Cronyx]". If the \a family is
    available from more than one foundry and the foundry isn't
    specified, an arbitrary foundry is chosen. If the family isn't
    available a family will be set using the \l{QFont}{font matching}
    algorithm.

    This will split the family string on a comma and call setFamilies() with the
    resulting list. To preserve a font that uses a comma in its name, use
    the constructor that takes a QStringList.

    \sa Weight, setFamily(), setPointSize(), setWeight(), setItalic(),
    setStyleHint(), setFamilies(), QGuiApplication::font()
*/
QFont::QFont(const QString &family, int pointSize, int weight, bool italic)
    : d(new QFontPrivate()), resolve_mask(QFont::FamiliesResolved)
{
    if (pointSize <= 0) {
        pointSize = 12;
    } else {
        resolve_mask |= QFont::SizeResolved;
    }

    if (weight < 0) {
        weight = Normal;
    } else {
        resolve_mask |= QFont::WeightResolved | QFont::StyleResolved;
    }

    if (italic)
        resolve_mask |= QFont::StyleResolved;

    d->request.families = splitIntoFamilies(family);
    d->request.pointSize = qreal(pointSize);
    d->request.pixelSize = -1;
    d->request.weight = weight;
    d->request.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
}

/*!
     Constructs a font object with the specified \a families, \a
     pointSize, \a weight and \a italic settings.

     If \a pointSize is zero or negative, the point size of the font
     is set to a system-dependent default value. Generally, this is
     12 points.

     Each family name entry in \a families may optionally also include
     a foundry name, e.g. "Helvetica [Cronyx]". If the family is
     available from more than one foundry and the foundry isn't
     specified, an arbitrary foundry is chosen. If the family isn't
     available a family will be set using the \l{QFont}{font matching}
     algorithm.

     \sa Weight, setPointSize(), setWeight(), setItalic(),
     setStyleHint(), setFamilies(), QGuiApplication::font()
 */
QFont::QFont(const QStringList &families, int pointSize, int weight, bool italic)
    : d(new QFontPrivate()), resolve_mask(QFont::FamiliesResolved)
{
    if (pointSize <= 0)
        pointSize = 12;
    else
        resolve_mask |= QFont::SizeResolved;

    if (weight < 0)
        weight = Normal;
    else
        resolve_mask |= QFont::WeightResolved | QFont::StyleResolved;

    if (italic)
        resolve_mask |= QFont::StyleResolved;

    d->request.families = families;
    d->request.pointSize = qreal(pointSize);
    d->request.pixelSize = -1;
    d->request.weight = weight;
    d->request.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
}

/*!
    Constructs a font that is a copy of \a font.
*/
QFont::QFont(const QFont &font)
    : d(font.d), resolve_mask(font.resolve_mask)
{
}

/*!
    Destroys the font object and frees all allocated resources.
*/
QFont::~QFont()
{
}

/*!
    Assigns \a font to this font and returns a reference to it.
*/
QFont &QFont::operator=(const QFont &font)
{
    d = font.d;
    resolve_mask = font.resolve_mask;
    return *this;
}

/*!
    \fn void QFont::swap(QFont &other)
    \since 5.0

    Swaps this font instance with \a other. This function is very fast
    and never fails.
*/

/*!
    Returns the requested font family name.  This will always be the same
    as the first entry in the families() call.

    \sa setFamily(), substitutes(), substitute(), setFamilies(), families()
*/
QString QFont::family() const
{
    return d->request.families.isEmpty() ? QString() : d->request.families.first();
}

/*!
    Sets the family name of the font. The name is case insensitive and
    may include a foundry name.

    The \a family name may optionally also include a foundry name,
    e.g. "Helvetica [Cronyx]". If the \a family is
    available from more than one foundry and the foundry isn't
    specified, an arbitrary foundry is chosen. If the family isn't
    available a family will be set using the \l{QFont}{font matching}
    algorithm.

    This will split the family string on a comma and call setFamilies() with the
    resulting list. To preserve a font that uses a comma in it's name then use
    setFamilies() directly. From Qt 6.2 this behavior will no longer happen and
    \a family will be passed as a single family.

    \sa family(), setStyleHint(), setFamilies(), families(), QFontInfo
*/
void QFont::setFamily(const QString &family)
{
#ifdef QT_DEBUG
    if (family.contains(QLatin1Char(','))) {
        qWarning("From Qt 6.2, QFont::setFamily() will no long split the family string on the comma"
                 " and will keep it as a single family");
    }
#endif
    setFamilies(splitIntoFamilies(family));
}

/*!
    \since 4.8

    Returns the requested font style name. This can be used to match the
    font with irregular styles (that can't be normalized in other style
    properties).

    \sa setFamily(), setStyle()
*/
QString QFont::styleName() const
{
    return d->request.styleName;
}

/*!
    \since 4.8

    Sets the style name of the font to \a styleName. When set, other style properties
    like \l style() and \l weight() will be ignored for font matching, though they may be
    simulated afterwards if supported by the platform's font engine.

    Due to the lower quality of artificially simulated styles, and the lack of full cross
    platform support, it is not recommended to use matching by style name together with
    matching by style properties

    \sa styleName()
*/
void QFont::setStyleName(const QString &styleName)
{
    if ((resolve_mask & QFont::StyleNameResolved) && d->request.styleName == styleName)
        return;

    detach();

    d->request.styleName = styleName;
    resolve_mask |= QFont::StyleNameResolved;
}

/*!
    Returns the point size of the font. Returns -1 if the font size
    was specified in pixels.

    \sa setPointSize(), pointSizeF()
*/
int QFont::pointSize() const
{
    return qRound(d->request.pointSize);
}

/*!
    \since 4.8

    \enum QFont::HintingPreference

    This enum describes the different levels of hinting that can be applied
    to glyphs to improve legibility on displays where it might be warranted
    by the density of pixels.

    \value PreferDefaultHinting Use the default hinting level for the target platform.
    \value PreferNoHinting If possible, render text without hinting the outlines
           of the glyphs. The text layout will be typographically accurate and
           scalable, using the same metrics as are used e.g. when printing.
    \value PreferVerticalHinting If possible, render text with no horizontal hinting,
           but align glyphs to the pixel grid in the vertical direction. The text will appear
           crisper on displays where the density is too low to give an accurate rendering
           of the glyphs. But since the horizontal metrics of the glyphs are unhinted, the text's
           layout will be scalable to higher density devices (such as printers) without impacting
           details such as line breaks.
    \value PreferFullHinting If possible, render text with hinting in both horizontal and
           vertical directions. The text will be altered to optimize legibility on the target
           device, but since the metrics will depend on the target size of the text, the positions
           of glyphs, line breaks, and other typographical detail will not scale, meaning that a
           text layout may look different on devices with different pixel densities.

    Please note that this enum only describes a preference, as the full range of hinting levels
    are not supported on all of Qt's supported platforms. The following table details the effect
    of a given hinting preference on a selected set of target platforms.

    \table
    \header
    \li
    \li PreferDefaultHinting
    \li PreferNoHinting
    \li PreferVerticalHinting
    \li PreferFullHinting
    \row
    \li Windows and DirectWrite enabled in Qt
    \li Full hinting
    \li Vertical hinting
    \li Vertical hinting
    \li Full hinting
    \row
    \li FreeType
    \li Operating System setting
    \li No hinting
    \li Vertical hinting (light)
    \li Full hinting
    \row
    \li Cocoa on \macos
    \li No hinting
    \li No hinting
    \li No hinting
    \li No hinting
    \endtable

*/

/*!
    \since 4.8

    Set the preference for the hinting level of the glyphs to \a hintingPreference. This is a hint
    to the underlying font rendering system to use a certain level of hinting, and has varying
    support across platforms. See the table in the documentation for QFont::HintingPreference for
    more details.

    The default hinting preference is QFont::PreferDefaultHinting.
*/
void QFont::setHintingPreference(HintingPreference hintingPreference)
{
    if ((resolve_mask & QFont::HintingPreferenceResolved) && d->request.hintingPreference == hintingPreference)
        return;

    detach();

    d->request.hintingPreference = hintingPreference;

    resolve_mask |= QFont::HintingPreferenceResolved;
}

/*!
    \since 4.8

    Returns the currently preferred hinting level for glyphs rendered with this font.
*/
QFont::HintingPreference QFont::hintingPreference() const
{
    return QFont::HintingPreference(d->request.hintingPreference);
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero.

    \sa pointSize(), setPointSizeF()
*/
void QFont::setPointSize(int pointSize)
{
    if (pointSize <= 0) {
        qWarning("QFont::setPointSize: Point size <= 0 (%d), must be greater than 0", pointSize);
        return;
    }

    if ((resolve_mask & QFont::SizeResolved) && d->request.pointSize == qreal(pointSize))
        return;

    detach();

    d->request.pointSize = qreal(pointSize);
    d->request.pixelSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Sets the point size to \a pointSize. The point size must be
    greater than zero. The requested precision may not be achieved on
    all platforms.

    \sa pointSizeF(), setPointSize(), setPixelSize()
*/
void QFont::setPointSizeF(qreal pointSize)
{
    if (pointSize <= 0) {
        qWarning("QFont::setPointSizeF: Point size <= 0 (%f), must be greater than 0", pointSize);
        return;
    }

    if ((resolve_mask & QFont::SizeResolved) && d->request.pointSize == pointSize)
        return;

    detach();

    d->request.pointSize = pointSize;
    d->request.pixelSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Returns the point size of the font. Returns -1 if the font size was
    specified in pixels.

    \sa pointSize(), setPointSizeF(), pixelSize(), QFontInfo::pointSize(), QFontInfo::pixelSize()
*/
qreal QFont::pointSizeF() const
{
    return d->request.pointSize;
}

/*!
    Sets the font size to \a pixelSize pixels, with a maxiumum size
    of an unsigned 16-bit integer.

    Using this function makes the font device dependent. Use
    setPointSize() or setPointSizeF() to set the size of the font
    in a device independent manner.

    \sa pixelSize()
*/
void QFont::setPixelSize(int pixelSize)
{
    if (pixelSize <= 0) {
        qWarning("QFont::setPixelSize: Pixel size <= 0 (%d)", pixelSize);
        return;
    }

    if ((resolve_mask & QFont::SizeResolved) && d->request.pixelSize == qreal(pixelSize))
        return;

    detach();

    d->request.pixelSize = pixelSize;
    d->request.pointSize = -1;

    resolve_mask |= QFont::SizeResolved;
}

/*!
    Returns the pixel size of the font if it was set with
    setPixelSize(). Returns -1 if the size was set with setPointSize()
    or setPointSizeF().

    \sa setPixelSize(), pointSize(), QFontInfo::pointSize(), QFontInfo::pixelSize()
*/
int QFont::pixelSize() const
{
    return d->request.pixelSize;
}

/*!
  \fn bool QFont::italic() const

    Returns \c true if the style() of the font is not QFont::StyleNormal

    \sa setItalic(), style()
*/

/*!
  \fn void QFont::setItalic(bool enable)

  Sets the style() of the font to QFont::StyleItalic if \a enable is true;
  otherwise the style is set to QFont::StyleNormal.

  \note If styleName() is set, this value may be ignored, or if supported
  on the platform, the font may be rendered tilted instead of picking a
  designed italic font-variant.

  \sa italic(), QFontInfo
*/

/*!
    Returns the style of the font.

    \sa setStyle()
*/
QFont::Style QFont::style() const
{
    return (QFont::Style)d->request.style;
}


/*!
  Sets the style of the font to \a style.

  \sa italic(), QFontInfo
*/
void QFont::setStyle(Style style)
{
    if ((resolve_mask & QFont::StyleResolved) && d->request.style == style)
        return;

    detach();

    d->request.style = style;
    resolve_mask |= QFont::StyleResolved;
}

/*!
    Returns the weight of the font, using the same scale as the
    \l{QFont::Weight} enumeration.

    \sa setWeight(), Weight, QFontInfo
*/
QFont::Weight QFont::weight() const
{
    return static_cast<Weight>(d->request.weight);
}

/*!
    \enum QFont::Weight

    Qt uses a weighting scale from 1 to 1000 compatible with OpenType. A weight of 1 will be
    thin, whilst 1000 will be extremely black.

    This enum contains the predefined font weights:

    \value Thin 100
    \value ExtraLight 200
    \value Light 300
    \value Normal 400
    \value Medium 500
    \value DemiBold 600
    \value Bold 700
    \value ExtraBold 800
    \value Black 900
*/

#if QT_DEPRECATED_SINCE(6, 0)
/*!
    \deprecated [6.0] Use setWeight() instead.

    Sets the weight of the font to \a legacyWeight using the legacy font
    weight scale of Qt 5 and previous versions.

    Since Qt 6, the OpenType standard's font weight scale is used instead
    of a non-standard scale. This requires conversion from values that
    use the old scale. For convenience, this function may be used when
    porting from code which uses the old weight scale.

    \note If styleName() is set, this value may be ignored for font selection.

    \sa setWeight(), weight(), QFontInfo
*/
void QFont::setLegacyWeight(int legacyWeight)
{
    setWeight(QFont::Weight(qt_legacyToOpenTypeWeight(legacyWeight)));
}

/*!
    \deprecated [6.0] Use weight() instead.

    Returns the weight of the font converted to the non-standard font
    weight scale used in Qt 5 and earlier versions.

    Since Qt 6, the OpenType standard's font weight scale is used instead
    of a non-standard scale. This requires conversion from values that
    use the old scale. For convenience, this function may be used when
    porting from code which uses the old weight scale.

    \sa setWeight(), weight(), QFontInfo
*/
int QFont::legacyWeight() const
{
    return qt_openTypeToLegacyWeight(weight());
}
#endif // QT_DEPRECATED_SINCE(6, 0)

/*!
    Sets the weight of the font to \a weight, using the scale defined by
    \l QFont::Weight enumeration.

    \note If styleName() is set, this value may be ignored for font selection.

    \sa weight(), QFontInfo
*/
void QFont::setWeight(QFont::Weight weight)
{
    const int weightValue = qBound(QFONT_WEIGHT_MIN, static_cast<int>(weight), QFONT_WEIGHT_MAX);
    if (weightValue != static_cast<int>(weight)) {
        qWarning() << "QFont::setWeight: Weight must be between 1 and 1000, attempted to set "
                   << static_cast<int>(weight);
    }

    if ((resolve_mask & QFont::WeightResolved) && d->request.weight == weightValue)
        return;

    detach();

    d->request.weight = weightValue;
    resolve_mask |= QFont::WeightResolved;
}

/*!
    \fn bool QFont::bold() const

    Returns \c true if weight() is a value greater than
   \l{Weight}{QFont::Medium}; otherwise returns \c false.

    \sa weight(), setBold(), QFontInfo::bold()
*/

/*!
    \fn void QFont::setBold(bool enable)

    If \a enable is true sets the font's weight to
    \l{Weight}{QFont::Bold};
    otherwise sets the weight to \l{Weight}{QFont::Normal}.

    For finer boldness control use setWeight().

    \note If styleName() is set, this value may be ignored, or if supported
    on the platform, the font artificially embolded.

    \sa bold(), setWeight()
*/

/*!
    Returns \c true if underline has been set; otherwise returns \c false.

    \sa setUnderline()
*/
bool QFont::underline() const
{
    return d->underline;
}

/*!
    If \a enable is true, sets underline on; otherwise sets underline
    off.

    \sa underline(), QFontInfo
*/
void QFont::setUnderline(bool enable)
{
    if ((resolve_mask & QFont::UnderlineResolved) && d->underline == enable)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->underline = enable;
    resolve_mask |= QFont::UnderlineResolved;
}

/*!
    Returns \c true if overline has been set; otherwise returns \c false.

    \sa setOverline()
*/
bool QFont::overline() const
{
    return d->overline;
}

/*!
  If \a enable is true, sets overline on; otherwise sets overline off.

  \sa overline(), QFontInfo
*/
void QFont::setOverline(bool enable)
{
    if ((resolve_mask & QFont::OverlineResolved) && d->overline == enable)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->overline = enable;
    resolve_mask |= QFont::OverlineResolved;
}

/*!
    Returns \c true if strikeout has been set; otherwise returns \c false.

    \sa setStrikeOut()
*/
bool QFont::strikeOut() const
{
    return d->strikeOut;
}

/*!
    If \a enable is true, sets strikeout on; otherwise sets strikeout
    off.

    \sa strikeOut(), QFontInfo
*/
void QFont::setStrikeOut(bool enable)
{
    if ((resolve_mask & QFont::StrikeOutResolved) && d->strikeOut == enable)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->strikeOut = enable;
    resolve_mask |= QFont::StrikeOutResolved;
}

/*!
    Returns \c true if fixed pitch has been set; otherwise returns \c false.

    \sa setFixedPitch(), QFontInfo::fixedPitch()
*/
bool QFont::fixedPitch() const
{
    return d->request.fixedPitch;
}

/*!
    If \a enable is true, sets fixed pitch on; otherwise sets fixed
    pitch off.

    \sa fixedPitch(), QFontInfo
*/
void QFont::setFixedPitch(bool enable)
{
    if ((resolve_mask & QFont::FixedPitchResolved) && d->request.fixedPitch == enable)
        return;

    detach();

    d->request.fixedPitch = enable;
    d->request.ignorePitch = false;
    resolve_mask |= QFont::FixedPitchResolved;
}

/*!
  Returns \c true if kerning should be used when drawing text with this font.

  \sa setKerning()
*/
bool QFont::kerning() const
{
    return d->kerning;
}

/*!
    Enables kerning for this font if \a enable is true; otherwise
    disables it. By default, kerning is enabled.

    When kerning is enabled, glyph metrics do not add up anymore,
    even for Latin text. In other words, the assumption that
    width('a') + width('b') is equal to width("ab") is not
    necessarily true.

    \sa kerning(), QFontMetrics
*/
void QFont::setKerning(bool enable)
{
    if ((resolve_mask & QFont::KerningResolved) && d->kerning == enable)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->kerning = enable;
    resolve_mask |= QFont::KerningResolved;
}

/*!
    Returns the StyleStrategy.

    The style strategy affects the \l{QFont}{font matching} algorithm.
    See \l QFont::StyleStrategy for the list of available strategies.

    \sa setStyleHint(), QFont::StyleHint
*/
QFont::StyleStrategy QFont::styleStrategy() const
{
    return (StyleStrategy) d->request.styleStrategy;
}

/*!
    Returns the StyleHint.

    The style hint affects the \l{QFont#fontmatching}{font matching algorithm}.
    See \l QFont::StyleHint for the list of available hints.

    \sa setStyleHint(), QFont::StyleStrategy, QFontInfo::styleHint()
*/
QFont::StyleHint QFont::styleHint() const
{
    return (StyleHint) d->request.styleHint;
}

/*!
    \enum QFont::StyleHint

    Style hints are used by the \l{QFont}{font matching} algorithm to
    find an appropriate default family if a selected font family is
    not available.

    \value AnyStyle leaves the font matching algorithm to choose the
           family. This is the default.

    \value SansSerif the font matcher prefer sans serif fonts.
    \value Helvetica is a synonym for \c SansSerif.

    \value Serif the font matcher prefers serif fonts.
    \value Times is a synonym for \c Serif.

    \value TypeWriter the font matcher prefers fixed pitch fonts.
    \value Courier a synonym for \c TypeWriter.

    \value OldEnglish the font matcher prefers decorative fonts.
    \value Decorative is a synonym for \c OldEnglish.

    \value Monospace the font matcher prefers fonts that map to the
    CSS generic font-family 'monospace'.

    \value Fantasy the font matcher prefers fonts that map to the
    CSS generic font-family 'fantasy'.

    \value Cursive the font matcher prefers fonts that map to the
    CSS generic font-family 'cursive'.

    \value System the font matcher prefers system fonts.
*/

/*!
    \enum QFont::StyleStrategy

    The style strategy tells the \l{QFont}{font matching} algorithm
    what type of fonts should be used to find an appropriate default
    family.

    The following strategies are available:

    \value PreferDefault the default style strategy. It does not prefer
           any type of font.
    \value PreferBitmap prefers bitmap fonts (as opposed to outline
           fonts).
    \value PreferDevice prefers device fonts.
    \value PreferOutline prefers outline fonts (as opposed to bitmap fonts).
    \value ForceOutline forces the use of outline fonts.
    \value NoAntialias don't antialias the fonts.
    \value NoSubpixelAntialias avoid subpixel antialiasing on the fonts if possible.
    \value PreferAntialias antialias if possible.
    \value NoFontMerging If the font selected for a certain writing system
           does not contain a character requested to draw, then Qt automatically chooses a similar
           looking font that contains the character. The NoFontMerging flag disables this feature.
           Please note that enabling this flag will not prevent Qt from automatically picking a
           suitable font when the selected font does not support the writing system of the text.
    \value PreferNoShaping Sometimes, a font will apply complex rules to a set of characters in
           order to display them correctly. In some writing systems, such as Brahmic scripts, this is
           required in order for the text to be legible, but in e.g. Latin script, it is merely
           a cosmetic feature. The PreferNoShaping flag will disable all such features when they
           are not required, which will improve performance in most cases (since Qt 5.10).

    Any of these may be OR-ed with one of these flags:

    \value PreferMatch prefer an exact match. The font matcher will try to
           use the exact font size that has been specified.
    \value PreferQuality prefer the best quality font. The font matcher
           will use the nearest standard point size that the font
           supports.
*/

/*!
    Sets the style hint and strategy to \a hint and \a strategy,
    respectively.

    If these aren't set explicitly the style hint will default to
    \c AnyStyle and the style strategy to \c PreferDefault.

    Qt does not support style hints on X11 since this information
    is not provided by the window system.

    \sa StyleHint, styleHint(), StyleStrategy, styleStrategy(), QFontInfo
*/
void QFont::setStyleHint(StyleHint hint, StyleStrategy strategy)
{
    if ((resolve_mask & (QFont::StyleHintResolved | QFont::StyleStrategyResolved)) &&
         (StyleHint) d->request.styleHint == hint &&
         (StyleStrategy) d->request.styleStrategy == strategy)
        return;

    detach();

    d->request.styleHint = hint;
    d->request.styleStrategy = strategy;
    resolve_mask |= QFont::StyleHintResolved;
    resolve_mask |= QFont::StyleStrategyResolved;

}

/*!
    Sets the style strategy for the font to \a s.

    \sa QFont::StyleStrategy
*/
void QFont::setStyleStrategy(StyleStrategy s)
{
    if ((resolve_mask & QFont::StyleStrategyResolved) &&
         s == (StyleStrategy)d->request.styleStrategy)
        return;

    detach();

    d->request.styleStrategy = s;
    resolve_mask |= QFont::StyleStrategyResolved;
}


/*!
    \enum QFont::Stretch

    Predefined stretch values that follow the CSS naming convention. The higher
    the value, the more stretched the text is.

    \value AnyStretch 0 Accept any stretch matched using the other QFont properties (added in Qt 5.8)
    \value UltraCondensed 50
    \value ExtraCondensed 62
    \value Condensed 75
    \value SemiCondensed 87
    \value Unstretched 100
    \value SemiExpanded 112
    \value Expanded 125
    \value ExtraExpanded 150
    \value UltraExpanded 200

    \sa setStretch(), stretch()
*/

/*!
    Returns the stretch factor for the font.

    \sa setStretch()
 */
int QFont::stretch() const
{
    return d->request.stretch;
}

/*!
    Sets the stretch factor for the font.

    The stretch factor matches a condensed or expanded version of the font or
    applies a stretch transform that changes the width of all characters
    in the font by \a factor percent.  For example, setting \a factor to 150
    results in all characters in the font being 1.5 times (ie. 150%)
    wider.  The minimum stretch factor is 1, and the maximum stretch factor
    is 4000.  The default stretch factor is \c AnyStretch, which will accept
    any stretch factor and not apply any transform on the font.

    The stretch factor is only applied to outline fonts.  The stretch
    factor is ignored for bitmap fonts.

    \note When matching a font with a native non-default stretch factor,
    requesting a stretch of 100 will stretch it back to a medium width font.

    \sa stretch(), QFont::Stretch
*/
void QFont::setStretch(int factor)
{
    if (factor < 0 || factor > 4000) {
        qWarning("QFont::setStretch: Parameter '%d' out of range", factor);
        return;
    }

    if ((resolve_mask & QFont::StretchResolved) &&
         d->request.stretch == (uint)factor)
        return;

    detach();

    d->request.stretch = (uint)factor;
    resolve_mask |= QFont::StretchResolved;
}

/*!
    \enum QFont::SpacingType
    \since 4.4

    \value PercentageSpacing  A value of 100 will keep the spacing unchanged; a value of 200 will enlarge the
                                                   spacing after a character by the width of the character itself.
    \value AbsoluteSpacing      A positive value increases the letter spacing by the corresponding pixels; a negative
                                                   value decreases the spacing.
*/

/*!
    \since 4.4
    Returns the letter spacing for the font.

    \sa setLetterSpacing(), letterSpacingType(), setWordSpacing()
 */
qreal QFont::letterSpacing() const
{
    return d->letterSpacing.toReal();
}

/*!
    \since 4.4
    Sets the letter spacing for the font to \a spacing and the type
    of spacing to \a type.

    Letter spacing changes the default spacing between individual
    letters in the font.  The spacing between the letters can be
    made smaller as well as larger either in percentage of the
    character width or in pixels, depending on the selected spacing type.

    \sa letterSpacing(), letterSpacingType(), setWordSpacing()
*/
void QFont::setLetterSpacing(SpacingType type, qreal spacing)
{
    const QFixed newSpacing = QFixed::fromReal(spacing);
    const bool absoluteSpacing = type == AbsoluteSpacing;
    if ((resolve_mask & QFont::LetterSpacingResolved) &&
        d->letterSpacingIsAbsolute == absoluteSpacing &&
        d->letterSpacing == newSpacing)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->letterSpacing = newSpacing;
    d->letterSpacingIsAbsolute = absoluteSpacing;
    resolve_mask |= QFont::LetterSpacingResolved;
}

/*!
    \since 4.4
    Returns the spacing type used for letter spacing.

    \sa letterSpacing(), setLetterSpacing(), setWordSpacing()
*/
QFont::SpacingType QFont::letterSpacingType() const
{
    return d->letterSpacingIsAbsolute ? AbsoluteSpacing : PercentageSpacing;
}

/*!
    \since 4.4
    Returns the word spacing for the font.

    \sa setWordSpacing(), setLetterSpacing()
 */
qreal QFont::wordSpacing() const
{
    return d->wordSpacing.toReal();
}

/*!
    \since 4.4
    Sets the word spacing for the font to \a spacing.

    Word spacing changes the default spacing between individual
    words. A positive value increases the word spacing
    by a corresponding amount of pixels, while a negative value
    decreases the inter-word spacing accordingly.

    Word spacing will not apply to writing systems, where indiviaul
    words are not separated by white space.

    \sa wordSpacing(), setLetterSpacing()
*/
void QFont::setWordSpacing(qreal spacing)
{
    const QFixed newSpacing = QFixed::fromReal(spacing);
    if ((resolve_mask & QFont::WordSpacingResolved) &&
        d->wordSpacing == newSpacing)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->wordSpacing = newSpacing;
    resolve_mask |= QFont::WordSpacingResolved;
}

/*!
    \enum QFont::Capitalization
    \since 4.4

    Rendering option for text this font applies to.


    \value MixedCase    This is the normal text rendering option where no capitalization change is applied.
    \value AllUppercase This alters the text to be rendered in all uppercase type.
    \value AllLowercase This alters the text to be rendered in all lowercase type.
    \value SmallCaps    This alters the text to be rendered in small-caps type.
    \value Capitalize   This alters the text to be rendered with the first character of each word as an uppercase character.
*/

/*!
    \since 4.4
    Sets the capitalization of the text in this font to \a caps.

    A font's capitalization makes the text appear in the selected capitalization mode.

    \sa capitalization()
*/
void QFont::setCapitalization(Capitalization caps)
{
    if ((resolve_mask & QFont::CapitalizationResolved) &&
        capitalization() == caps)
        return;

    QFontPrivate::detachButKeepEngineData(this);

    d->capital = caps;
    resolve_mask |= QFont::CapitalizationResolved;
}

/*!
    \since 4.4
    Returns the current capitalization type of the font.

    \sa setCapitalization()
*/
QFont::Capitalization QFont::capitalization() const
{
    return static_cast<QFont::Capitalization> (d->capital);
}

/*!
    Returns \c true if a window system font exactly matching the settings
    of this font is available.

    \sa QFontInfo
*/
bool QFont::exactMatch() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return d->request.exactMatch(engine->fontDef);
}

/*!
    Returns \c true if this font is equal to \a f; otherwise returns
    false.

    Two QFonts are considered equal if their font attributes are
    equal.

    \sa operator!=(), isCopyOf()
*/
bool QFont::operator==(const QFont &f) const
{
    return (f.d == d
            || (f.d->request   == d->request
                && f.d->request.pointSize == d->request.pointSize
                && f.d->underline == d->underline
                && f.d->overline  == d->overline
                && f.d->strikeOut == d->strikeOut
                && f.d->kerning == d->kerning
                && f.d->capital == d->capital
                && f.d->letterSpacingIsAbsolute == d->letterSpacingIsAbsolute
                && f.d->letterSpacing == d->letterSpacing
                && f.d->wordSpacing == d->wordSpacing
            ));
}


/*!
    Provides an arbitrary comparison of this font and font \a f.
    All that is guaranteed is that the operator returns \c false if both
    fonts are equal and that (f1 \< f2) == !(f2 \< f1) if the fonts
    are not equal.

    This function is useful in some circumstances, for example if you
    want to use QFont objects as keys in a QMap.

    \sa operator==(), operator!=(), isCopyOf()
*/
bool QFont::operator<(const QFont &f) const
{
    if (f.d == d) return false;
    // the < operator for fontdefs ignores point sizes.
    const QFontDef &r1 = f.d->request;
    const QFontDef &r2 = d->request;
    if (r1.pointSize != r2.pointSize) return r1.pointSize < r2.pointSize;
    if (r1.pixelSize != r2.pixelSize) return r1.pixelSize < r2.pixelSize;
    if (r1.weight != r2.weight) return r1.weight < r2.weight;
    if (r1.style != r2.style) return r1.style < r2.style;
    if (r1.stretch != r2.stretch) return r1.stretch < r2.stretch;
    if (r1.styleHint != r2.styleHint) return r1.styleHint < r2.styleHint;
    if (r1.styleStrategy != r2.styleStrategy) return r1.styleStrategy < r2.styleStrategy;
    if (r1.families != r2.families) return r1.families < r2.families;
    if (f.d->capital != d->capital) return f.d->capital < d->capital;

    if (f.d->letterSpacingIsAbsolute != d->letterSpacingIsAbsolute) return f.d->letterSpacingIsAbsolute < d->letterSpacingIsAbsolute;
    if (f.d->letterSpacing != d->letterSpacing) return f.d->letterSpacing < d->letterSpacing;
    if (f.d->wordSpacing != d->wordSpacing) return f.d->wordSpacing < d->wordSpacing;

    int f1attrs = (f.d->underline << 3) + (f.d->overline << 2) + (f.d->strikeOut<<1) + f.d->kerning;
    int f2attrs = (d->underline << 3) + (d->overline << 2) + (d->strikeOut<<1) + d->kerning;
    return f1attrs < f2attrs;
}


/*!
    Returns \c true if this font is different from \a f; otherwise
    returns \c false.

    Two QFonts are considered to be different if their font attributes
    are different.

    \sa operator==()
*/
bool QFont::operator!=(const QFont &f) const
{
    return !(operator==(f));
}

/*!
   Returns the font as a QVariant
*/
QFont::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    Returns \c true if this font and \a f are copies of each other, i.e.
    one of them was created as a copy of the other and neither has
    been modified since. This is much stricter than equality.

    \sa operator=(), operator==()
*/
bool QFont::isCopyOf(const QFont & f) const
{
    return d == f.d;
}

/*!
    Returns a new QFont that has attributes copied from \a other that
    have not been previously set on this font.
*/
QFont QFont::resolve(const QFont &other) const
{
    if (resolve_mask == 0 || (resolve_mask == other.resolve_mask && *this == other)) {
        QFont o(other);
        o.resolve_mask = resolve_mask;
        return o;
    }

    QFont font(*this);
    font.detach();
    font.d->resolve(resolve_mask, other.d.data());

    return font;
}

/*!
    \fn uint QFont::resolveMask() const
    \internal
*/

/*!
    \fn void QFont::setResolveMask(uint mask)
    \internal
*/


/*****************************************************************************
  QFont substitution management
 *****************************************************************************/

typedef QHash<QString, QStringList> QFontSubst;
Q_GLOBAL_STATIC(QFontSubst, globalFontSubst)

/*!
    Returns the first family name to be used whenever \a familyName is
    specified. The lookup is case insensitive.

    If there is no substitution for \a familyName, \a familyName is
    returned.

    To obtain a list of substitutions use substitutes().

    \sa setFamily(), insertSubstitutions(), insertSubstitution(), removeSubstitutions()
*/
QString QFont::substitute(const QString &familyName)
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    QFontSubst::ConstIterator it = fontSubst->constFind(familyName.toLower());
    if (it != fontSubst->constEnd() && !(*it).isEmpty())
        return (*it).first();

    return familyName;
}


/*!
    Returns a list of family names to be used whenever \a familyName
    is specified. The lookup is case insensitive.

    If there is no substitution for \a familyName, an empty list is
    returned.

    \sa substitute(), insertSubstitutions(), insertSubstitution(), removeSubstitutions()
 */
QStringList QFont::substitutes(const QString &familyName)
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    return fontSubst->value(familyName.toLower(), QStringList());
}


/*!
    Inserts \a substituteName into the substitution
    table for the family \a familyName.

    After substituting a font, trigger the updating of the font by destroying
    and re-creating all QFont objects.

    \sa insertSubstitutions(), removeSubstitutions(), substitutions(), substitute(), substitutes()
*/
void QFont::insertSubstitution(const QString &familyName,
                               const QString &substituteName)
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    QStringList &list = (*fontSubst)[familyName.toLower()];
    QString s = substituteName.toLower();
    if (!list.contains(s))
        list.append(s);
}


/*!
    Inserts the list of families \a substituteNames into the
    substitution list for \a familyName.

    After substituting a font, trigger the updating of the font by destroying
    and re-creating all QFont objects.


    \sa insertSubstitution(), removeSubstitutions(), substitutions(), substitute()
*/
void QFont::insertSubstitutions(const QString &familyName,
                                const QStringList &substituteNames)
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    QStringList &list = (*fontSubst)[familyName.toLower()];
    for (const QString &substituteName : substituteNames) {
        const QString lowerSubstituteName = substituteName.toLower();
        if (!list.contains(lowerSubstituteName))
            list.append(lowerSubstituteName);
    }
}

/*!
    Removes all the substitutions for \a familyName.

    \sa insertSubstitutions(), insertSubstitution(), substitutions(), substitute()
    \since 5.0
*/
void QFont::removeSubstitutions(const QString &familyName)
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    fontSubst->remove(familyName.toLower());
}

/*!
    Returns a sorted list of substituted family names.

    \sa insertSubstitution(), removeSubstitutions(), substitute()
*/
QStringList QFont::substitutions()
{
    QFontSubst *fontSubst = globalFontSubst();
    Q_ASSERT(fontSubst != nullptr);
    QStringList ret = fontSubst->keys();

    ret.sort();
    return ret;
}

#ifndef QT_NO_DATASTREAM
/*  \internal
    Internal function. Converts boolean font settings to an unsigned
    8-bit number. Used for serialization etc.
*/
static quint8 get_font_bits(int version, const QFontPrivate *f)
{
    Q_ASSERT(f != nullptr);
    quint8 bits = 0;
    if (f->request.style)
        bits |= 0x01;
    if (f->underline)
        bits |= 0x02;
    if (f->overline)
        bits |= 0x40;
    if (f->strikeOut)
        bits |= 0x04;
    if (f->request.fixedPitch)
        bits |= 0x08;
    // if (f.hintSetByUser)
    // bits |= 0x10;
    if (version >= QDataStream::Qt_4_0) {
        if (f->kerning)
            bits |= 0x10;
    }
    if (f->request.style == QFont::StyleOblique)
        bits |= 0x80;
    return bits;
}

static quint8 get_extended_font_bits(const QFontPrivate *f)
{
    Q_ASSERT(f != nullptr);
    quint8 bits = 0;
    if (f->request.ignorePitch)
        bits |= 0x01;
    if (f->letterSpacingIsAbsolute)
        bits |= 0x02;
    return bits;
}

/*  \internal
    Internal function. Sets boolean font settings from an unsigned
    8-bit number. Used for serialization etc.
*/
static void set_font_bits(int version, quint8 bits, QFontPrivate *f)
{
    Q_ASSERT(f != nullptr);
    f->request.style         = (bits & 0x01) != 0 ? QFont::StyleItalic : QFont::StyleNormal;
    f->underline             = (bits & 0x02) != 0;
    f->overline              = (bits & 0x40) != 0;
    f->strikeOut             = (bits & 0x04) != 0;
    f->request.fixedPitch    = (bits & 0x08) != 0;
    // f->hintSetByUser      = (bits & 0x10) != 0;
    if (version >= QDataStream::Qt_4_0)
        f->kerning               = (bits & 0x10) != 0;
    if ((bits & 0x80) != 0)
        f->request.style         = QFont::StyleOblique;
}

static void set_extended_font_bits(quint8 bits, QFontPrivate *f)
{
    Q_ASSERT(f != nullptr);
    f->request.ignorePitch = (bits & 0x01) != 0;
    f->letterSpacingIsAbsolute = (bits & 0x02) != 0;
}
#endif

/*!
    Returns the font's key, a textual representation of a font. It is
    typically used as the key for a cache or dictionary of fonts.

    \sa QMap
*/
QString QFont::key() const
{
    return toString();
}

/*!
    Returns a description of the font. The description is a
    comma-separated list of the attributes, perfectly suited for use
    in QSettings, and consists of the following:

    \list
      \li Font family
      \li Point size
      \li Pixel size
      \li Style hint
      \li Font weight
      \li Font style
      \li Underline
      \li Strike out
      \li Fixed pitch
      \li Always \e{0}
      \li Capitalization
      \li Letter spacing
      \li Word spacing
      \li Stretch
      \li Style strategy
      \li Font style (omitted when unavailable)
    \endlist

    \sa fromString()
 */
QString QFont::toString() const
{
    const QChar comma(QLatin1Char(','));
    QString fontDescription = family() + comma +
        QString::number(     pointSizeF()) + comma +
        QString::number(      pixelSize()) + comma +
        QString::number((int) styleHint()) + comma +
        QString::number(         weight()) + comma +
        QString::number((int)     style()) + comma +
        QString::number((int) underline()) + comma +
        QString::number((int) strikeOut()) + comma +
        QString::number((int)fixedPitch()) + comma +
        QString::number((int)   false) + comma +
        QString::number((int)capitalization()) + comma +
        QString::number((int)letterSpacingType()) + comma +
        QString::number(letterSpacing()) + comma +
        QString::number(wordSpacing()) + comma +
        QString::number(stretch()) + comma +
        QString::number((int)styleStrategy());

    QString fontStyle = styleName();
    if (!fontStyle.isEmpty())
        fontDescription += comma + fontStyle;

    return fontDescription;
}

/*!
    Returns the hash value for \a font. If specified, \a seed is used
    to initialize the hash.

    \relates QFont
    \since 5.3
*/
size_t qHash(const QFont &font, size_t seed) noexcept
{
    return qHash(QFontPrivate::get(font)->request, seed);
}


/*!
    Sets this font to match the description \a descrip. The description
    is a comma-separated list of the font attributes, as returned by
    toString().

    \sa toString()
 */
bool QFont::fromString(const QString &descrip)
{
    const auto sr = QStringView(descrip).trimmed();
    const auto l = sr.split(QLatin1Char(','));
    const int count = l.count();
    if (!count || (count > 2 && count < 9) || count == 9 || count > 17 ||
        l.first().isEmpty()) {
        qWarning("QFont::fromString: Invalid description '%s'",
                 descrip.isEmpty() ? "(empty)" : descrip.toLatin1().data());
        return false;
    }

    setFamily(l[0].toString());
    if (count > 1 && l[1].toDouble() > 0.0)
        setPointSizeF(l[1].toDouble());
    if (count == 9) {
        setStyleHint((StyleHint) l[2].toInt());
        setWeight(QFont::Weight(l[3].toInt()));
        setItalic(l[4].toInt());
        setUnderline(l[5].toInt());
        setStrikeOut(l[6].toInt());
        setFixedPitch(l[7].toInt());
    } else if (count >= 10) {
        if (l[2].toInt() > 0)
            setPixelSize(l[2].toInt());
        setStyleHint((StyleHint) l[3].toInt());
        if (count >= 16)
            setWeight(QFont::Weight(l[4].toInt()));
        else
            setWeight(QFont::Weight(qt_legacyToOpenTypeWeight(l[4].toInt())));
        setStyle((QFont::Style)l[5].toInt());
        setUnderline(l[6].toInt());
        setStrikeOut(l[7].toInt());
        setFixedPitch(l[8].toInt());
        if (count >= 16) {
            setCapitalization((Capitalization)l[10].toInt());
            setLetterSpacing((SpacingType)l[11].toInt(), l[12].toDouble());
            setWordSpacing(l[13].toDouble());
            setStretch(l[14].toInt());
            setStyleStrategy((StyleStrategy)l[15].toInt());
        }
        if (count == 11 || count == 17)
            d->request.styleName = l[count - 1].toString();
        else
            d->request.styleName.clear();
    }

    if (count >= 9 && !d->request.fixedPitch) // assume 'false' fixedPitch equals default
        d->request.ignorePitch = true;

    return true;
}

/*! \fn void QFont::initialize()
  \internal

  Internal function that initializes the font system.  The font cache
  and font dict do not alloc the keys. The key is a QString which is
  shared between QFontPrivate and QXFontName.
*/
void QFont::initialize()
{
}

/*! \fn void QFont::cleanup()
  \internal

  Internal function that cleans up the font system.
*/
void QFont::cleanup()
{
    QFontCache::cleanup();
}

/*! \internal

  Internal function that dumps font cache statistics.
*/
void QFont::cacheStatistics()
{
}


extern QStringList qt_fallbacksForFamily(const QString &family, QFont::Style style,
                                         QFont::StyleHint styleHint, QChar::Script script);

/*!
    \fn QString QFont::defaultFamily() const

    Returns the family name that corresponds to the current style
    hint.

    \sa StyleHint, styleHint(), setStyleHint()
*/
QString QFont::defaultFamily() const
{
    const QStringList fallbacks = qt_fallbacksForFamily(QString(), QFont::StyleNormal
                                      , QFont::StyleHint(d->request.styleHint), QChar::Script_Common);
    if (!fallbacks.isEmpty())
        return fallbacks.first();
    return QString();
}

/*!
    \since 5.13

    Returns the requested font family names, i.e. the names set in the last
    setFamilies() call or via the constructor. Otherwise it returns an
    empty list.

    \sa setFamily(), setFamilies(), family(), substitutes(), substitute()
*/

QStringList QFont::families() const
{
    return d->request.families;
}

/*!
    \since 5.13

    Sets the list of family names for the font. The names are case
    insensitive and may include a foundry name. The first family in
    \a families will be set as the main family for the font.

    Each family name entry in \a families may optionally also include a
    foundry name, e.g. "Helvetica [Cronyx]". If the family is
    available from more than one foundry and the foundry isn't
    specified, an arbitrary foundry is chosen. If the family isn't
    available a family will be set using the \l{QFont}{font matching}
    algorithm.

    \sa family(), families(), setFamily(), setStyleHint(), QFontInfo
*/

void QFont::setFamilies(const QStringList &families)
{
    if ((resolve_mask & QFont::FamiliesResolved) && d->request.families == families)
        return;
    detach();
    d->request.families = families;
    resolve_mask |= QFont::FamiliesResolved;
}


/*****************************************************************************
  QFont stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM

/*!
    \relates QFont

    Writes the font \a font to the data stream \a s. (toString()
    writes to a text stream.)

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator<<(QDataStream &s, const QFont &font)
{
    if (s.version() == 1) {
        s << font.d->request.families.first().toLatin1();
    } else {
        s << font.d->request.families.first();
        if (s.version() >= QDataStream::Qt_5_4)
            s << font.d->request.styleName;
    }

    if (s.version() >= QDataStream::Qt_4_0) {
        // 4.0
        double pointSize = font.d->request.pointSize;
        qint32 pixelSize = font.d->request.pixelSize;
        s << pointSize;
        s << pixelSize;
    } else if (s.version() <= 3) {
        qint16 pointSize = (qint16) (font.d->request.pointSize * 10);
        if (pointSize < 0) {
            pointSize = (qint16)QFontInfo(font).pointSize() * 10;
        }
        s << pointSize;
    } else {
        s << (qint16) (font.d->request.pointSize * 10);
        s << (qint16) font.d->request.pixelSize;
    }

    s << (quint8) font.d->request.styleHint;
    if (s.version() >= QDataStream::Qt_3_1) {
        // Continue writing 8 bits for versions < 5.4 so that we don't write too much,
        // even though we need 16 to store styleStrategy, so there is some data loss.
        if (s.version() >= QDataStream::Qt_5_4)
            s << (quint16) font.d->request.styleStrategy;
        else
            s << (quint8) font.d->request.styleStrategy;
    }

    if (s.version() < QDataStream::Qt_6_0)
        s << quint8(0) << quint8(qt_openTypeToLegacyWeight(font.d->request.weight));
    else
        s << quint16(font.d->request.weight);

    s << get_font_bits(s.version(), font.d.data());
    if (s.version() >= QDataStream::Qt_4_3)
        s << (quint16)font.d->request.stretch;
    if (s.version() >= QDataStream::Qt_4_4)
        s << get_extended_font_bits(font.d.data());
    if (s.version() >= QDataStream::Qt_4_5) {
        s << font.d->letterSpacing.value();
        s << font.d->wordSpacing.value();
    }
    if (s.version() >= QDataStream::Qt_5_4)
        s << (quint8)font.d->request.hintingPreference;
    if (s.version() >= QDataStream::Qt_5_6)
        s << (quint8)font.d->capital;
    if (s.version() >= QDataStream::Qt_5_13) {
        if (s.version() < QDataStream::Qt_6_0)
            s << font.d->request.families.mid(1);
        else
            s << font.d->request.families;
    }
    return s;
}


/*!
    \relates QFont

    Reads the font \a font from the data stream \a s. (fromString()
    reads from a text stream.)

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator>>(QDataStream &s, QFont &font)
{
    font.d = new QFontPrivate;
    font.resolve_mask = QFont::AllPropertiesResolved;

    quint8 styleHint, bits;
    quint16 styleStrategy = QFont::PreferDefault;

    if (s.version() == 1) {
        QByteArray fam;
        s >> fam;
        font.d->request.families = QStringList(QString::fromLatin1(fam));
    } else {
        QString fam;
        s >> fam;
        font.d->request.families = QStringList(fam);
        if (s.version() >= QDataStream::Qt_5_4)
            s >> font.d->request.styleName;
    }

    if (s.version() >= QDataStream::Qt_4_0) {
        // 4.0
        double pointSize;
        qint32 pixelSize;
        s >> pointSize;
        s >> pixelSize;
        font.d->request.pointSize = qreal(pointSize);
        font.d->request.pixelSize = pixelSize;
    } else {
        qint16 pointSize, pixelSize = -1;
        s >> pointSize;
        if (s.version() >= 4)
            s >> pixelSize;
        font.d->request.pointSize = qreal(pointSize / 10.);
        font.d->request.pixelSize = pixelSize;
    }
    s >> styleHint;
    if (s.version() >= QDataStream::Qt_3_1) {
        if (s.version() >= QDataStream::Qt_5_4) {
            s >> styleStrategy;
        } else {
            quint8 tempStyleStrategy;
            s >> tempStyleStrategy;
            styleStrategy = tempStyleStrategy;
        }
    }

    if (s.version() < QDataStream::Qt_6_0) {
        quint8 charSet;
        quint8 weight;
        s >> charSet;
        s >> weight;
        font.d->request.weight = qt_legacyToOpenTypeWeight(weight);
    } else {
        quint16 weight;
        s >> weight;
        font.d->request.weight = weight;
    }

    s >> bits;

    font.d->request.styleHint = styleHint;
    font.d->request.styleStrategy = styleStrategy;

    set_font_bits(s.version(), bits, font.d.data());

    if (s.version() >= QDataStream::Qt_4_3) {
        quint16 stretch;
        s >> stretch;
        font.d->request.stretch = stretch;
    }

    if (s.version() >= QDataStream::Qt_4_4) {
        quint8 extendedBits;
        s >> extendedBits;
        set_extended_font_bits(extendedBits, font.d.data());
    }
    if (s.version() >= QDataStream::Qt_4_5) {
        int value;
        s >> value;
        font.d->letterSpacing.setValue(value);
        s >> value;
        font.d->wordSpacing.setValue(value);
    }
    if (s.version() >= QDataStream::Qt_5_4) {
        quint8 value;
        s >> value;
        font.d->request.hintingPreference = QFont::HintingPreference(value);
    }
    if (s.version() >= QDataStream::Qt_5_6) {
        quint8 value;
        s >> value;
        font.d->capital = QFont::Capitalization(value);
    }
    if (s.version() >= QDataStream::Qt_5_13) {
        QStringList value;
        s >> value;
        if (s.version() < QDataStream::Qt_6_0)
            font.d->request.families.append(value);
        else
            font.d->request.families = value;
    }
    return s;
}

#endif // QT_NO_DATASTREAM


/*****************************************************************************
  QFontInfo member functions
 *****************************************************************************/

/*!
    \class QFontInfo
    \reentrant

    \brief The QFontInfo class provides general information about fonts.
    \inmodule QtGui

    \ingroup appearance
    \ingroup shared

    The QFontInfo class provides the same access functions as QFont,
    e.g. family(), pointSize(), italic(), weight(), fixedPitch(),
    styleHint() etc. But whilst the QFont access functions return the
    values that were set, a QFontInfo object returns the values that
    apply to the font that will actually be used to draw the text.

    For example, when the program asks for a 25pt Courier font on a
    machine that has a non-scalable 24pt Courier font, QFont will
    (normally) use the 24pt Courier for rendering. In this case,
    QFont::pointSize() returns 25 and QFontInfo::pointSize() returns
    24.

    There are three ways to create a QFontInfo object.
    \list 1
    \li Calling the QFontInfo constructor with a QFont creates a font
    info object for a screen-compatible font, i.e. the font cannot be
    a printer font. If the font is changed later, the font
    info object is \e not updated.

    (Note: If you use a printer font the values returned may be
    inaccurate. Printer fonts are not always accessible so the nearest
    screen font is used if a printer font is supplied.)

    \li QWidget::fontInfo() returns the font info for a widget's font.
    This is equivalent to calling QFontInfo(widget->font()). If the
    widget's font is changed later, the font info object is \e not
    updated.

    \li QPainter::fontInfo() returns the font info for a painter's
    current font. If the painter's font is changed later, the font
    info object is \e not updated.
    \endlist

    \sa QFont, QFontMetrics, QFontDatabase
*/

/*!
    Constructs a font info object for \a font.

    The font must be screen-compatible, i.e. a font you use when
    drawing text in \l{QWidget}{widgets} or \l{QPixmap}{pixmaps}, not QPicture or QPrinter.

    The font info object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

    Use QPainter::fontInfo() to get the font info when painting.
    This will give correct results also when painting on paint device
    that is not screen-compatible.
*/
QFontInfo::QFontInfo(const QFont &font)
    : d(font.d)
{
}

/*!
    Constructs a copy of \a fi.
*/
QFontInfo::QFontInfo(const QFontInfo &fi)
    : d(fi.d)
{
}

/*!
    Destroys the font info object.
*/
QFontInfo::~QFontInfo()
{
}

/*!
    Assigns the font info in \a fi.
*/
QFontInfo &QFontInfo::operator=(const QFontInfo &fi)
{
    d = fi.d;
    return *this;
}

/*!
    \fn void QFontInfo::swap(QFontInfo &other)
    \since 5.0

    Swaps this font info instance with \a other. This function is very
    fast and never fails.
*/

/*!
    Returns the family name of the matched window system font.

    \sa QFont::family()
*/
QString QFontInfo::family() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.families.isEmpty() ? QString() : engine->fontDef.families.first();
}

/*!
    \since 4.8

    Returns the style name of the matched window system font on
    systems that support it.

    \sa QFont::styleName()
*/
QString QFontInfo::styleName() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.styleName;
}

/*!
    Returns the point size of the matched window system font.

    \sa pointSizeF(), QFont::pointSize()
*/
int QFontInfo::pointSize() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return qRound(engine->fontDef.pointSize);
}

/*!
    Returns the point size of the matched window system font.

    \sa QFont::pointSizeF()
*/
qreal QFontInfo::pointSizeF() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.pointSize;
}

/*!
    Returns the pixel size of the matched window system font.

    \sa QFont::pointSize()
*/
int QFontInfo::pixelSize() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.pixelSize;
}

/*!
    Returns the italic value of the matched window system font.

    \sa QFont::italic()
*/
bool QFontInfo::italic() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.style != QFont::StyleNormal;
}

/*!
    Returns the style value of the matched window system font.

    \sa QFont::style()
*/
QFont::Style QFontInfo::style() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return (QFont::Style)engine->fontDef.style;
}


#if QT_DEPRECATED_SINCE(6, 0)
/*!
    \deprecated Use weight() instead.

    Returns the weight of the font converted to the non-standard font
    weight scale used in Qt 5 and earlier versions.

    Since Qt 6, the OpenType standard's font weight scale is used instead
    of a non-standard scale. This requires conversion from values that
    use the old scale. For convenience, this function may be used when
    porting from code which uses the old weight scale.

    \sa QFont::setWeight(), weight(), QFontInfo
*/
int QFontInfo::legacyWeight() const
{
    return qt_openTypeToLegacyWeight(weight());
}
#endif // QT_DEPRECATED_SINCE(6, 0)


/*!
    Returns the weight of the matched window system font.

    \sa QFont::weight(), bold()
*/
int QFontInfo::weight() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return engine->fontDef.weight;

}

/*!
    \fn bool QFontInfo::bold() const

    Returns \c true if weight() would return a value greater than
    QFont::Normal; otherwise returns \c false.

    \sa weight(), QFont::bold()
*/

/*!
    Returns the underline value of the matched window system font.

  \sa QFont::underline()

  \internal

  Here we read the underline flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::underline() const
{
    return d->underline;
}

/*!
    Returns the overline value of the matched window system font.

    \sa QFont::overline()

    \internal

    Here we read the overline flag directly from the QFont.
    This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::overline() const
{
    return d->overline;
}

/*!
    Returns the strikeout value of the matched window system font.

  \sa QFont::strikeOut()

  \internal Here we read the strikeOut flag directly from the QFont.
  This is OK for X11 and for Windows because we always get what we want.
*/
bool QFontInfo::strikeOut() const
{
    return d->strikeOut;
}

/*!
    Returns the fixed pitch value of the matched window system font.

    \sa QFont::fixedPitch()
*/
bool QFontInfo::fixedPitch() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
#ifdef Q_OS_MAC
    if (!engine->fontDef.fixedPitchComputed) {
        QChar ch[2] = { QLatin1Char('i'), QLatin1Char('m') };
        QGlyphLayoutArray<2> g;
        int l = 2;
        if (!engine->stringToCMap(ch, 2, &g, &l, {}))
            Q_UNREACHABLE();
        Q_ASSERT(l == 2);
        engine->fontDef.fixedPitch = g.advances[0] == g.advances[1];
        engine->fontDef.fixedPitchComputed = true;
    }
#endif
    return engine->fontDef.fixedPitch;
}

/*!
    Returns the style of the matched window system font.

    Currently only returns the style hint set in QFont.

    \sa QFont::styleHint(), QFont::StyleHint
*/
QFont::StyleHint QFontInfo::styleHint() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return (QFont::StyleHint) engine->fontDef.styleHint;
}

/*!
    Returns \c true if the matched window system font is exactly the same
    as the one specified by the font; otherwise returns \c false.

    \sa QFont::exactMatch()
*/
bool QFontInfo::exactMatch() const
{
    QFontEngine *engine = d->engineForScript(QChar::Script_Common);
    Q_ASSERT(engine != nullptr);
    return d->request.exactMatch(engine->fontDef);
}




// **********************************************************************
// QFontCache
// **********************************************************************

#ifdef QFONTCACHE_DEBUG
// fast timeouts for debugging
static const int fast_timeout =   1000;  // 1s
static const int slow_timeout =   5000;  // 5s
#else
static const int fast_timeout =  10000; // 10s
static const int slow_timeout = 300000; //  5m
#endif // QFONTCACHE_DEBUG

#ifndef QFONTCACHE_MIN_COST
#  define QFONTCACHE_MIN_COST 4*1024 // 4mb
#endif
const uint QFontCache::min_cost = QFONTCACHE_MIN_COST;
Q_GLOBAL_STATIC(QThreadStorage<QFontCache *>, theFontCache)

QFontCache *QFontCache::instance()
{
    QFontCache *&fontCache = theFontCache()->localData();
    if (!fontCache)
        fontCache = new QFontCache;
    return fontCache;
}

void QFontCache::cleanup()
{
    QThreadStorage<QFontCache *> *cache = nullptr;
    QT_TRY {
        cache = theFontCache();
    } QT_CATCH (const std::bad_alloc &) {
        // no cache - just ignore
    }
    if (cache && cache->hasLocalData())
        cache->setLocalData(nullptr);
}

static QBasicAtomicInt font_cache_id = Q_BASIC_ATOMIC_INITIALIZER(0);

QFontCache::QFontCache()
    : QObject(), total_cost(0), max_cost(min_cost),
      current_timestamp(0), fast(false),
      autoClean(QGuiApplication::instance()
                && (QGuiApplication::instance()->thread() == QThread::currentThread())),
      timer_id(-1),
      m_id(font_cache_id.fetchAndAddRelaxed(1) + 1)
{
}

QFontCache::~QFontCache()
{
    clear();
}

void QFontCache::clear()
{
    {
        EngineDataCache::Iterator it = engineDataCache.begin(),
                                 end = engineDataCache.end();
        while (it != end) {
            QFontEngineData *data = it.value();
            for (int i = 0; i < QChar::ScriptCount; ++i) {
                if (data->engines[i]) {
                    if (!data->engines[i]->ref.deref()) {
                        Q_ASSERT(engineCacheCount.value(data->engines[i]) == 0);
                        delete data->engines[i];
                    }
                    data->engines[i] = nullptr;
                }
            }
            if (!data->ref.deref()) {
                delete data;
            } else {
                FC_DEBUG("QFontCache::clear: engineData %p still has refcount %d",
                         data, data->ref.loadRelaxed());
            }
            ++it;
        }
    }

    engineDataCache.clear();


    bool mightHaveEnginesLeftForCleanup;
    do {
        mightHaveEnginesLeftForCleanup = false;
        for (EngineCache::Iterator it = engineCache.begin(), end = engineCache.end();
             it != end; ++it) {
            QFontEngine *engine = it.value().data;
            if (engine) {
                const int cacheCount = --engineCacheCount[engine];
                Q_ASSERT(cacheCount >= 0);
                if (!engine->ref.deref()) {
                    Q_ASSERT(cacheCount == 0);
                    mightHaveEnginesLeftForCleanup = engine->type() == QFontEngine::Multi;
                    delete engine;
                } else if (cacheCount == 0) {
                    FC_DEBUG("QFontCache::clear: engine %p still has refcount %d",
                             engine, engine->ref.loadRelaxed());
                }
                it.value().data = nullptr;
            }
        }
    } while (mightHaveEnginesLeftForCleanup);

    engineCache.clear();
    engineCacheCount.clear();


    total_cost = 0;
    max_cost = min_cost;
}


QFontEngineData *QFontCache::findEngineData(const QFontDef &def) const
{
    EngineDataCache::ConstIterator it = engineDataCache.constFind(def);
    if (it == engineDataCache.constEnd())
        return nullptr;

    // found
    return it.value();
}

void QFontCache::insertEngineData(const QFontDef &def, QFontEngineData *engineData)
{
#ifdef QFONTCACHE_DEBUG
    FC_DEBUG("QFontCache: inserting new engine data %p", engineData);
    if (engineDataCache.contains(def)) {
        FC_DEBUG("   QFontCache already contains engine data %p for key=(%g %g %d %d %d)",
                 engineDataCache.value(def), def.pointSize,
                 def.pixelSize, def.weight, def.style, def.fixedPitch);
    }
#endif
    Q_ASSERT(!engineDataCache.contains(def));

    engineData->ref.ref();
    // Decrease now rather than waiting
    if (total_cost > min_cost * 2 && engineDataCache.size() >= QFONTCACHE_DECREASE_TRIGGER_LIMIT)
        decreaseCache();

    engineDataCache.insert(def, engineData);
    increaseCost(sizeof(QFontEngineData));
}

QFontEngine *QFontCache::findEngine(const Key &key)
{
    EngineCache::Iterator it = engineCache.find(key),
                         end = engineCache.end();
    if (it == end) return nullptr;

    Q_ASSERT(it.value().data != nullptr);
    Q_ASSERT(key.multi == (it.value().data->type() == QFontEngine::Multi));

    // found... update the hitcount and timestamp
    updateHitCountAndTimeStamp(it.value());

    return it.value().data;
}

void QFontCache::updateHitCountAndTimeStamp(Engine &value)
{
    value.hits++;
    value.timestamp = ++current_timestamp;

    FC_DEBUG("QFontCache: found font engine\n"
             "  %p: timestamp %4u hits %3u ref %2d/%2d, type %d",
             value.data, value.timestamp, value.hits,
             value.data->ref.loadRelaxed(), engineCacheCount.value(value.data),
             value.data->type());
}

void QFontCache::insertEngine(const Key &key, QFontEngine *engine, bool insertMulti)
{
    Q_ASSERT(engine != nullptr);
    Q_ASSERT(key.multi == (engine->type() == QFontEngine::Multi));

#ifdef QFONTCACHE_DEBUG
    FC_DEBUG("QFontCache: inserting new engine %p, refcount %d", engine, engine->ref.loadRelaxed());
    if (!insertMulti && engineCache.contains(key)) {
        FC_DEBUG("   QFontCache already contains engine %p for key=(%g %g %d %d %d)",
                 engineCache.value(key).data, key.def.pointSize,
                 key.def.pixelSize, key.def.weight, key.def.style, key.def.fixedPitch);
    }
#endif
    engine->ref.ref();
    // Decrease now rather than waiting
    if (total_cost > min_cost * 2 && engineCache.size() >= QFONTCACHE_DECREASE_TRIGGER_LIMIT)
        decreaseCache();

    Engine data(engine);
    data.timestamp = ++current_timestamp;

    if (insertMulti)
        engineCache.insert(key, data);
    else
        engineCache.replace(key, data);
    // only increase the cost if this is the first time we insert the engine
    if (++engineCacheCount[engine] == 1)
        increaseCost(engine->cache_cost);
}

void QFontCache::increaseCost(uint cost)
{
    cost = (cost + 512) / 1024; // store cost in kb
    cost = cost > 0 ? cost : 1;
    total_cost += cost;

    FC_DEBUG("  COST: increased %u kb, total_cost %u kb, max_cost %u kb",
            cost, total_cost, max_cost);

    if (total_cost > max_cost) {
        max_cost = total_cost;

        if (!autoClean)
            return;

        if (timer_id == -1 || ! fast) {
            FC_DEBUG("  TIMER: starting fast timer (%d ms)", fast_timeout);

            if (timer_id != -1)
                killTimer(timer_id);
            timer_id = startTimer(fast_timeout);
            fast = true;
        }
    }
}

void QFontCache::decreaseCost(uint cost)
{
    cost = (cost + 512) / 1024; // cost is stored in kb
    cost = cost > 0 ? cost : 1;
    Q_ASSERT(cost <= total_cost);
    total_cost -= cost;

    FC_DEBUG("  COST: decreased %u kb, total_cost %u kb, max_cost %u kb",
            cost, total_cost, max_cost);
}

void QFontCache::timerEvent(QTimerEvent *)
{
    FC_DEBUG("QFontCache::timerEvent: performing cache maintenance (timestamp %u)",
              current_timestamp);

    if (total_cost <= max_cost && max_cost <= min_cost) {
        FC_DEBUG("  cache redused sufficiently, stopping timer");

        killTimer(timer_id);
        timer_id = -1;
        fast = false;

        return;
    }
    decreaseCache();
}

void QFontCache::decreaseCache()
{
    // go through the cache and count up everything in use
    uint in_use_cost = 0;

    {
        FC_DEBUG("  SWEEP engine data:");

        // make sure the cost of each engine data is at least 1kb
        const uint engine_data_cost =
            sizeof(QFontEngineData) > 1024 ? sizeof(QFontEngineData) : 1024;

        EngineDataCache::ConstIterator it = engineDataCache.constBegin(),
                                      end = engineDataCache.constEnd();
        for (; it != end; ++it) {
            FC_DEBUG("    %p: ref %2d", it.value(), int(it.value()->ref.loadRelaxed()));

            if (it.value()->ref.loadRelaxed() != 1)
                in_use_cost += engine_data_cost;
        }
    }

    {
        FC_DEBUG("  SWEEP engine:");

        EngineCache::ConstIterator it = engineCache.constBegin(),
                                  end = engineCache.constEnd();
        for (; it != end; ++it) {
            FC_DEBUG("    %p: timestamp %4u hits %2u ref %2d/%2d, cost %u bytes",
                     it.value().data, it.value().timestamp, it.value().hits,
                     it.value().data->ref.loadRelaxed(), engineCacheCount.value(it.value().data),
                     it.value().data->cache_cost);

            if (it.value().data->ref.loadRelaxed() > engineCacheCount.value(it.value().data))
                in_use_cost += it.value().data->cache_cost / engineCacheCount.value(it.value().data);
        }

        // attempt to make up for rounding errors
        in_use_cost += engineCache.size();
    }

    in_use_cost = (in_use_cost + 512) / 1024; // cost is stored in kb

    /*
      calculate the new maximum cost for the cache

      NOTE: in_use_cost is *not* correct due to rounding errors in the
      above algorithm.  instead of worrying about getting the
      calculation correct, we are more interested in speed, and use
      in_use_cost as a floor for new_max_cost
    */
    uint new_max_cost = qMax(qMax(max_cost / 2, in_use_cost), min_cost);

    FC_DEBUG("  after sweep, in use %u kb, total %u kb, max %u kb, new max %u kb",
              in_use_cost, total_cost, max_cost, new_max_cost);

    if (autoClean) {
        if (new_max_cost == max_cost) {
            if (fast) {
                FC_DEBUG("  cannot shrink cache, slowing timer");

                if (timer_id != -1) {
                    killTimer(timer_id);
                timer_id = startTimer(slow_timeout);
                fast = false;
            }

            return;
        } else if (! fast) {
            FC_DEBUG("  dropping into passing gear");

            if (timer_id != -1)
                killTimer(timer_id);
            timer_id = startTimer(fast_timeout);
            fast = true;        }
        }
    }

    max_cost = new_max_cost;

    {
        FC_DEBUG("  CLEAN engine data:");

        // clean out all unused engine data
        EngineDataCache::Iterator it = engineDataCache.begin();
        while (it != engineDataCache.end()) {
            if (it.value()->ref.loadRelaxed() == 1) {
                FC_DEBUG("    %p", it.value());
                decreaseCost(sizeof(QFontEngineData));
                it.value()->ref.deref();
                delete it.value();
                it = engineDataCache.erase(it);
            } else {
                ++it;
            }
        }
    }

    FC_DEBUG("  CLEAN engine:");

    // clean out the engine cache just enough to get below our new max cost
    bool cost_decreased;
    do {
        cost_decreased = false;

        EngineCache::Iterator it = engineCache.begin(),
                             end = engineCache.end();
        // determine the oldest and least popular of the unused engines
        uint oldest = ~0u;
        uint least_popular = ~0u;

        EngineCache::Iterator jt = end;

        for ( ; it != end; ++it) {
            if (it.value().data->ref.loadRelaxed() != engineCacheCount.value(it.value().data))
                continue;

            if (it.value().timestamp < oldest && it.value().hits <= least_popular) {
                oldest = it.value().timestamp;
                least_popular = it.value().hits;
                jt = it;
            }
        }

        it = jt;
        if (it != end) {
            FC_DEBUG("    %p: timestamp %4u hits %2u ref %2d/%2d, type %d",
                     it.value().data, it.value().timestamp, it.value().hits,
                     it.value().data->ref.loadRelaxed(), engineCacheCount.value(it.value().data),
                     it.value().data->type());

            QFontEngine *fontEngine = it.value().data;
            // get rid of all occurrences
            it = engineCache.begin();
            while (it != engineCache.end()) {
                if (it.value().data == fontEngine) {
                    fontEngine->ref.deref();
                    it = engineCache.erase(it);
                } else {
                    ++it;
                }
            }
            // and delete the last occurrence
            Q_ASSERT(fontEngine->ref.loadRelaxed() == 0);
            decreaseCost(fontEngine->cache_cost);
            delete fontEngine;
            engineCacheCount.remove(fontEngine);

            cost_decreased = true;
        }
    } while (cost_decreased && total_cost > max_cost);
}


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug stream, const QFont &font)
{
    QDebugStateSaver saver(stream);
    stream.nospace().noquote();
    stream << "QFont(";

    if (stream.verbosity() == QDebug::DefaultVerbosity) {
        stream << font.toString() << ")";
        return stream;
    }

    QString fontDescription;
    QDebug debug(&fontDescription);
    debug.nospace();

    const QFont defaultFont(new QFontPrivate);

    for (int property = QFont::SizeResolved; property < QFont::AllPropertiesResolved; property <<= 1) {
        const bool resolved = (font.resolve_mask & property) != 0;
        if (!resolved && stream.verbosity() == QDebug::MinimumVerbosity)
            continue;

        #define QFONT_DEBUG_SKIP_DEFAULT(prop) \
            if ((font.prop() == defaultFont.prop()) && stream.verbosity() == 1) \
                continue;

        QDebugStateSaver saver(debug);

        switch (property) {
        case QFont::SizeResolved:
            if (font.pointSizeF() >= 0)
                debug << font.pointSizeF() << "pt";
            else if (font.pixelSize() >= 0)
                debug << font.pixelSize() << "px";
            else
                Q_UNREACHABLE();
            break;
        case QFont::StyleHintResolved:
            QFONT_DEBUG_SKIP_DEFAULT(styleHint);
            debug.verbosity(1) << font.styleHint(); break;
        case QFont::StyleStrategyResolved:
            QFONT_DEBUG_SKIP_DEFAULT(styleStrategy);
            debug.verbosity(1) << font.styleStrategy(); break;
        case QFont::WeightResolved:
            debug.verbosity(1) << QFont::Weight(font.weight()); break;
        case QFont::StyleResolved:
            QFONT_DEBUG_SKIP_DEFAULT(style);
            debug.verbosity(0) << font.style(); break;
        case QFont::UnderlineResolved:
            QFONT_DEBUG_SKIP_DEFAULT(underline);
            debug << "underline=" << font.underline(); break;
        case QFont::OverlineResolved:
            QFONT_DEBUG_SKIP_DEFAULT(overline);
            debug << "overline=" << font.overline(); break;
        case QFont::StrikeOutResolved:
            QFONT_DEBUG_SKIP_DEFAULT(strikeOut);
            debug << "strikeOut=" << font.strikeOut(); break;
        case QFont::FixedPitchResolved:
            QFONT_DEBUG_SKIP_DEFAULT(fixedPitch);
            debug << "fixedPitch=" << font.fixedPitch(); break;
        case QFont::StretchResolved:
            QFONT_DEBUG_SKIP_DEFAULT(stretch);
            debug.verbosity(0) << QFont::Stretch(font.stretch()); break;
        case QFont::KerningResolved:
            QFONT_DEBUG_SKIP_DEFAULT(kerning);
            debug << "kerning=" << font.kerning(); break;
        case QFont::CapitalizationResolved:
            QFONT_DEBUG_SKIP_DEFAULT(capitalization);
            debug.verbosity(0) << font.capitalization(); break;
        case QFont::LetterSpacingResolved:
            QFONT_DEBUG_SKIP_DEFAULT(letterSpacing);
            debug << "letterSpacing=" << font.letterSpacing();
            debug.verbosity(0) << " (" << font.letterSpacingType() << ")";
            break;
        case QFont::HintingPreferenceResolved:
            QFONT_DEBUG_SKIP_DEFAULT(hintingPreference);
            debug.verbosity(0) << font.hintingPreference(); break;
        case QFont::StyleNameResolved:
            QFONT_DEBUG_SKIP_DEFAULT(styleName);
            debug << "styleName=" << font.styleName(); break;
        default:
            continue;
        };

        #undef QFONT_DEBUG_SKIP_DEFAULT

        debug << ", ";
    }

    if (stream.verbosity() > QDebug::MinimumVerbosity)
        debug.verbosity(0) << "resolveMask=" << QFlags<QFont::ResolveProperties>(font.resolve_mask);
    else
        fontDescription.chop(2); // Last ', '

    stream << fontDescription << ')';

    return stream;
}
#endif

QT_END_NAMESPACE

#include "moc_qfont.cpp"
