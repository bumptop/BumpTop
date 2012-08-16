/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <private/qapplication_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <qbitmap.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qprintengine_mac_p.h>
#include <qglobal.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qvarlengtharray.h>
#include <qdebug.h>
#include <qendian.h>
#include <qmath.h>

#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/AppKit.h>

QT_BEGIN_NAMESPACE

static float SYNTHETIC_ITALIC_SKEW = tanf(14 * acosf(0) / 90);

/*****************************************************************************
  QFontEngine debug facilities
 *****************************************************************************/
//#define DEBUG_ADVANCES

extern int qt_antialiasing_threshold; // QApplication.cpp

#ifndef FixedToQFixed
#define FixedToQFixed(a) QFixed::fromFixed((a) >> 10)
#define QFixedToFixed(x) ((x).value() << 10)
#endif

class QMacFontPath
{
    float x, y;
    QPainterPath *path;
public:
    inline QMacFontPath(float _x, float _y, QPainterPath *_path) : x(_x), y(_y), path(_path) { }
    inline void setPosition(float _x, float _y) { x = _x; y = _y; }
    inline void advance(float _x) { x += _x; }
    static OSStatus lineTo(const Float32Point *, void *);
    static OSStatus cubicTo(const Float32Point *, const Float32Point *,
                            const Float32Point *, void *);
    static OSStatus moveTo(const Float32Point *, void *);
    static OSStatus closePath(void *);
};

OSStatus QMacFontPath::lineTo(const Float32Point *pt, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->lineTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::cubicTo(const Float32Point *cp1, const Float32Point *cp2,
                               const Float32Point *ep, void *data)

{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->cubicTo(p->x + cp1->x, p->y + cp1->y,
                     p->x + cp2->x, p->y + cp2->y,
                     p->x + ep->x, p->y + ep->y);
    return noErr;
}

OSStatus QMacFontPath::moveTo(const Float32Point *pt, void *data)
{
    QMacFontPath *p = static_cast<QMacFontPath*>(data);
    p->path->moveTo(p->x + pt->x, p->y + pt->y);
    return noErr;
}

OSStatus QMacFontPath::closePath(void *data)
{
    static_cast<QMacFontPath*>(data)->path->closeSubpath();
    return noErr;
}



void qmacfontengine_gamma_correct(QImage *image)
{
    extern uchar qt_pow_rgb_gamma[256];

    // gamma correct the pixels back to linear color space...
    int h = image->height();
    int w = image->width();

    for (int y=0; y<h; ++y) {
        uint *pixels = (uint *) image->scanLine(y);
        for (int x=0; x<w; ++x) {
            uint p = pixels[x];
            uint r = qt_pow_rgb_gamma[qRed(p)];
            uint g = qt_pow_rgb_gamma[qGreen(p)];
            uint b = qt_pow_rgb_gamma[qBlue(p)];
            pixels[x] = (r << 16) | (g << 8) | b | 0xff000000;
        }
    }
}


#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
QCoreTextFontEngineMulti::QCoreTextFontEngineMulti(const ATSFontFamilyRef &atsFamilyRef, const ATSFontRef &atsFontRef, const QFontDef &fontDef, bool kerning)
    : QFontEngineMulti(0)
{
    this->fontDef = fontDef;
    CTFontSymbolicTraits symbolicTraits = 0;
    if (fontDef.weight >= QFont::Bold)
        symbolicTraits |= kCTFontBoldTrait;
    switch (fontDef.style) {
    case QFont::StyleNormal:
        break;
    case QFont::StyleItalic:
    case QFont::StyleOblique:
        symbolicTraits |= kCTFontItalicTrait;
        break;
    }

    QCFString name;
    ATSFontGetName(atsFontRef, kATSOptionFlagsDefault, &name);
    if (QString(name).isEmpty())
        ATSFontFamilyGetName(atsFamilyRef, kATSOptionFlagsDefault, &name);

    transform = CGAffineTransformIdentity;
    if (fontDef.stretch != 100) {
        transform = CGAffineTransformMakeScale(float(fontDef.stretch) / float(100), 1);
    }

    QCFType<CTFontDescriptorRef> descriptor = CTFontDescriptorCreateWithNameAndSize(name, fontDef.pixelSize);
    QCFType<CTFontRef> baseFont = CTFontCreateWithFontDescriptor(descriptor, fontDef.pixelSize, &transform);
    ctfont = CTFontCreateCopyWithSymbolicTraits(baseFont, fontDef.pixelSize, &transform, symbolicTraits, symbolicTraits);

    // CTFontCreateCopyWithSymbolicTraits returns NULL if we ask for a trait that does
    // not exist for the given font. (for example italic)
    if (ctfont == 0) {
        ctfont = baseFont;
        CFRetain(ctfont);
    }

    attributeDict = CFDictionaryCreateMutable(0, 2,
                                       &kCFTypeDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(attributeDict, NSFontAttributeName, ctfont);
    if (!kerning) {
        float zero = 0.0;
        QCFType<CFNumberRef> noKern = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &zero);
        CFDictionaryAddValue(attributeDict, kCTKernAttributeName, noKern);
    }

    QCoreTextFontEngine *fe = new QCoreTextFontEngine(ctfont, fontDef, this);
    fe->ref.ref();
    engines.append(fe);

}

QCoreTextFontEngineMulti::~QCoreTextFontEngineMulti()
{
    CFRelease(ctfont);
}

uint QCoreTextFontEngineMulti::fontIndexForFont(CTFontRef id) const
{
    for (int i = 0; i < engines.count(); ++i) {
        if (CFEqual(engineAt(i)->ctfont, id))
            return i;
    }

    QCoreTextFontEngineMulti *that = const_cast<QCoreTextFontEngineMulti *>(this);
    QCoreTextFontEngine *fe = new QCoreTextFontEngine(id, fontDef, that);
    fe->ref.ref();
    that->engines.append(fe);
    return engines.count() - 1;
}

bool QCoreTextFontEngineMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags,
                  unsigned short *logClusters, const HB_CharAttributes *) const
{
    QCFType<CFStringRef> cfstring = CFStringCreateWithCharactersNoCopy(0,
                                                               reinterpret_cast<const UniChar *>(str),
                                                               len, kCFAllocatorNull);
    QCFType<CFAttributedStringRef> attributedString = CFAttributedStringCreate(0, cfstring, attributeDict);
    QCFType<CTTypesetterRef> typeSetter = CTTypesetterCreateWithAttributedString(attributedString);
    CFRange range = {0, 0};
    QCFType<CTLineRef> line = CTTypesetterCreateLine(typeSetter, range);
    CFArrayRef array = CTLineGetGlyphRuns(line);
    uint arraySize = CFArrayGetCount(array);
    glyph_t *outGlyphs = glyphs->glyphs;
    HB_GlyphAttributes *outAttributes = glyphs->attributes;
    QFixed *outAdvances_x = glyphs->advances_x;
    QFixed *outAdvances_y = glyphs->advances_y;
    glyph_t *initialGlyph = outGlyphs;

    if (arraySize == 0) {
        // CoreText failed to shape the text we gave it, so we assume one glyph
        // per character and build a list of invalid glyphs with zero advance
        *nglyphs = len;
        for (int i = 0; i < len; ++i) {
            outGlyphs[i] = 0;
            if (logClusters)
                logClusters[i] = i;
            outAdvances_x[i] = QFixed();
            outAdvances_y[i] = QFixed();
            outAttributes[i].clusterStart = true;
        }
        return true;
    }

    const bool rtl = (CTRunGetStatus(static_cast<CTRunRef>(CFArrayGetValueAtIndex(array, 0))) & kCTRunStatusRightToLeft);

    bool outOBounds = false;
    for (uint i = 0; i < arraySize; ++i) {
        CTRunRef run = static_cast<CTRunRef>(CFArrayGetValueAtIndex(array, rtl ? (arraySize - 1 - i) : i));
        CFIndex glyphCount = CTRunGetGlyphCount(run);
        if (glyphCount == 0)
            continue;

        Q_ASSERT((CTRunGetStatus(run) & kCTRunStatusRightToLeft) == rtl);

        if (!outOBounds && outGlyphs + glyphCount - initialGlyph > *nglyphs) {
            outOBounds = true;
        }
        if (!outOBounds) {
            CFDictionaryRef runAttribs = CTRunGetAttributes(run);
            //NSLog(@"Dictionary %@", runAttribs);
            if (!runAttribs)
                runAttribs = attributeDict;
            CTFontRef runFont = static_cast<CTFontRef>(CFDictionaryGetValue(runAttribs, NSFontAttributeName));
            const uint fontIndex = (fontIndexForFont(runFont) << 24);
            //NSLog(@"Run Font Name = %@", CTFontCopyFamilyName(runFont));
            QVarLengthArray<CGGlyph, 512> cgglyphs(0);
            const CGGlyph *tmpGlyphs = CTRunGetGlyphsPtr(run);
            if (!tmpGlyphs) {
                cgglyphs.resize(glyphCount);
                CTRunGetGlyphs(run, range, cgglyphs.data());
                tmpGlyphs = cgglyphs.constData();
            }
            QVarLengthArray<CGPoint, 512> cgpoints(0);
            const CGPoint *tmpPoints = CTRunGetPositionsPtr(run);
            if (!tmpPoints) {
                cgpoints.resize(glyphCount);
                CTRunGetPositions(run, range, cgpoints.data());
                tmpPoints = cgpoints.constData();
            }

            const int rtlOffset = rtl ? (glyphCount - 1) : 0;
            const int rtlSign = rtl ? -1 : 1;

            if (logClusters) {
                CFRange stringRange = CTRunGetStringRange(run);
                QVarLengthArray<CFIndex, 512> stringIndices(0);
                const CFIndex *tmpIndices = CTRunGetStringIndicesPtr(run);
                if (!tmpIndices) {
                    stringIndices.resize(glyphCount);
                    CTRunGetStringIndices(run, range, stringIndices.data());
                    tmpIndices = stringIndices.constData();
                }

                const int firstGlyphIndex = outGlyphs - initialGlyph;
                outAttributes[0].clusterStart = true;

                CFIndex k = 0;
                CFIndex i = 0;
                for (i = stringRange.location;
                     (i < stringRange.location + stringRange.length) && (k < glyphCount); ++i) {
                    if (tmpIndices[k * rtlSign + rtlOffset] == i || i == stringRange.location) {
                        logClusters[i] = k + firstGlyphIndex;
                        outAttributes[k].clusterStart = true;
                        ++k;
                    } else {
                        logClusters[i] = k + firstGlyphIndex - 1;
                    }
                }
                // in case of a ligature at the end, fill the remaining logcluster entries
                for (;i < stringRange.location + stringRange.length; i++) {
                    logClusters[i] = k + firstGlyphIndex - 1;
                }
            }
            for (CFIndex i = 0; i < glyphCount - 1; ++i) {
                int idx = rtlOffset + rtlSign * i;
                outGlyphs[idx] = tmpGlyphs[i] | fontIndex;
                outAdvances_x[idx] = QFixed::fromReal(tmpPoints[i + 1].x - tmpPoints[i].x);
                outAdvances_y[idx] = QFixed::fromReal(tmpPoints[i + 1].y - tmpPoints[i].y);
                
                if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
                    outAdvances_x[idx] = outAdvances_x[idx].round();
                    outAdvances_y[idx] = outAdvances_y[idx].round();
                }
            }
            CGSize lastGlyphAdvance;
            CTFontGetAdvancesForGlyphs(runFont, kCTFontHorizontalOrientation, tmpGlyphs + glyphCount - 1, &lastGlyphAdvance, 1);

            outGlyphs[rtl ? 0 : (glyphCount - 1)] = tmpGlyphs[glyphCount - 1] | fontIndex;
            outAdvances_x[rtl ? 0 : (glyphCount - 1)] =
                    (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
                    ? QFixed::fromReal(lastGlyphAdvance.width).round()
                    : QFixed::fromReal(lastGlyphAdvance.width);
        }
        outGlyphs += glyphCount;
        outAttributes += glyphCount;
        outAdvances_x += glyphCount;
        outAdvances_y += glyphCount;
    }
    *nglyphs = (outGlyphs - initialGlyph);
    return !outOBounds;
}

bool QCoreTextFontEngineMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs,
                                            int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    *nglyphs = len;
    QCFType<CFStringRef> cfstring;

    QVarLengthArray<CGGlyph> cgGlyphs(len);
    CTFontGetGlyphsForCharacters(ctfont, (const UniChar*)str, cgGlyphs.data(), len);

    for (int i = 0; i < len; ++i) {
        if (cgGlyphs[i]) {
            glyphs->glyphs[i] = cgGlyphs[i];
	} else {
            if (!cfstring)
                cfstring = CFStringCreateWithCharactersNoCopy(0, reinterpret_cast<const UniChar *>(str), len, kCFAllocatorNull);
            QCFType<CTFontRef> substituteFont = CTFontCreateForString(ctfont, cfstring, CFRangeMake(i, 1));
            CGGlyph substituteGlyph = 0;
            CTFontGetGlyphsForCharacters(substituteFont, (const UniChar*)str + i, &substituteGlyph, 1);
            if (substituteGlyph) {
                const uint fontIndex = (fontIndexForFont(substituteFont) << 24);
                glyphs->glyphs[i] = substituteGlyph | fontIndex;
                if (!(flags & QTextEngine::GlyphIndicesOnly)) {
                    CGSize advance;
                    CTFontGetAdvancesForGlyphs(substituteFont, kCTFontHorizontalOrientation, &substituteGlyph, &advance, 1);
                    glyphs->advances_x[i] = QFixed::fromReal(advance.width);
                    glyphs->advances_y[i] = QFixed::fromReal(advance.height);
                }
            }
        }
    }

    if (flags & QTextEngine::GlyphIndicesOnly)
        return true;

    QVarLengthArray<CGSize> advances(len);
    CTFontGetAdvancesForGlyphs(ctfont, kCTFontHorizontalOrientation, cgGlyphs.data(), advances.data(), len);

    for (int i = 0; i < len; ++i) {
        if (glyphs->glyphs[i] & 0xff000000)
            continue;
        glyphs->advances_x[i] = QFixed::fromReal(advances[i].width);
        glyphs->advances_y[i] = QFixed::fromReal(advances[i].height);
    }

    if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
        for (int i = 0; i < len; ++i) {
            glyphs->advances_x[i] = glyphs->advances_x[i].round();
            glyphs->advances_y[i] = glyphs->advances_y[i].round();
        }
    }

    return true;
}

void QCoreTextFontEngineMulti::recalcAdvances(int , QGlyphLayout *, QTextEngine::ShaperFlags) const
{
}
void QCoreTextFontEngineMulti::doKerning(int , QGlyphLayout *, QTextEngine::ShaperFlags) const
{
}

void QCoreTextFontEngineMulti::loadEngine(int)
{
    // Do nothing
    Q_ASSERT(false);
}



QCoreTextFontEngine::QCoreTextFontEngine(CTFontRef font, const QFontDef &def,
                                         QCoreTextFontEngineMulti *multiEngine)
{
    fontDef = def;
    parentEngine = multiEngine;
    synthesisFlags = 0;
    ctfont = font;
    CFRetain(ctfont);
    ATSFontRef atsfont = CTFontGetPlatformFont(ctfont, 0);
    cgFont = CGFontCreateWithPlatformFont(&atsfont);
    CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(ctfont);
    if (fontDef.weight >= QFont::Bold && !(traits & kCTFontBoldTrait)) {
         synthesisFlags |= SynthesizedBold;
    }

    if (fontDef.style != QFont::StyleNormal && !(traits & kCTFontItalicTrait)) {
        synthesisFlags |= SynthesizedItalic;
    }
    transform = CGAffineTransformIdentity;
    if (fontDef.stretch != 100) {
        transform = CGAffineTransformMakeScale(float(fontDef.stretch) / float(100), 1);
    }
    QByteArray os2Table = getSfntTable(MAKE_TAG('O', 'S', '/', '2'));
    if (os2Table.size() >= 10)
        fsType = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(os2Table.constData() + 8));
}

QCoreTextFontEngine::~QCoreTextFontEngine()
{
    CFRelease(ctfont);
    CFRelease(cgFont);
}

bool QCoreTextFontEngine::stringToCMap(const QChar *, int, QGlyphLayout *, int *, QTextEngine::ShaperFlags) const
{
    return false;
}

glyph_metrics_t QCoreTextFontEngine::boundingBox(const QGlyphLayout &glyphs)
{
    QFixed w;
    bool round = fontDef.styleStrategy & QFont::ForceIntegerMetrics;

    for (int i = 0; i < glyphs.numGlyphs; ++i) {
        w += round ? glyphs.effectiveAdvance(i).round()
                   : glyphs.effectiveAdvance(i);
    }
    return glyph_metrics_t(0, -(ascent()), w - lastRightBearing(glyphs, round), ascent()+descent(), w, 0);
}
glyph_metrics_t QCoreTextFontEngine::boundingBox(glyph_t glyph)
{
    glyph_metrics_t ret;
    CGGlyph g = glyph;
    CGRect rect = CTFontGetBoundingRectsForGlyphs(ctfont, kCTFontHorizontalOrientation, &g, 0, 1);
    if (synthesisFlags & QFontEngine::SynthesizedItalic) {
        rect.size.width += rect.size.height * SYNTHETIC_ITALIC_SKEW;
    }
    ret.width = QFixed::fromReal(rect.size.width);
    ret.height = QFixed::fromReal(rect.size.height);
    ret.x = QFixed::fromReal(rect.origin.x);
    ret.y = -QFixed::fromReal(rect.origin.y) - ret.height;
    CGSize advances[1];
    CTFontGetAdvancesForGlyphs(ctfont, kCTFontHorizontalOrientation, &g, advances, 1);
    ret.xoff = QFixed::fromReal(advances[0].width);
    ret.yoff = QFixed::fromReal(advances[0].height);

    if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
        ret.xoff = ret.xoff.round();
        ret.yoff = ret.yoff.round();
    }

    return ret;
}

QFixed QCoreTextFontEngine::ascent() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? QFixed::fromReal(CTFontGetAscent(ctfont)).round()
            : QFixed::fromReal(CTFontGetAscent(ctfont));
}
QFixed QCoreTextFontEngine::descent() const
{
    QFixed d = QFixed::fromReal(CTFontGetDescent(ctfont));
    if (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
        d = d.round();

    // subtract a pixel to even out the historical +1 in QFontMetrics::height().
    // Fix in Qt 5.
    return d - 1;
}
QFixed QCoreTextFontEngine::leading() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? QFixed::fromReal(CTFontGetLeading(ctfont)).round()
            : QFixed::fromReal(CTFontGetLeading(ctfont));
}
QFixed QCoreTextFontEngine::xHeight() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? QFixed::fromReal(CTFontGetXHeight(ctfont)).round()
            : QFixed::fromReal(CTFontGetXHeight(ctfont));
}
QFixed QCoreTextFontEngine::averageCharWidth() const
{
    // ### Need to implement properly and get the information from the OS/2 Table.
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? QFontEngine::averageCharWidth().round()
            : QFontEngine::averageCharWidth();
}

qreal QCoreTextFontEngine::maxCharWidth() const
{
    // ### Max Help!
    return 0;

}
qreal QCoreTextFontEngine::minLeftBearing() const
{
    // ### Min Help!
    return 0;

}
qreal QCoreTextFontEngine::minRightBearing() const
{
    // ### Max Help! (even thought it's right)
    return 0;

}

void QCoreTextFontEngine::draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight)
{
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix;
    matrix.translate(x, y);
    getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    CGContextSetFontSize(ctx, fontDef.pixelSize);

    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);

    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, -1, 0, -paintDeviceHeight);

    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -SYNTHETIC_ITALIC_SKEW, 1, 0, 0));

    cgMatrix = CGAffineTransformConcat(cgMatrix, transform);

    CGContextSetTextMatrix(ctx, cgMatrix);

    CGContextSetTextDrawingMode(ctx, kCGTextFill);


    QVarLengthArray<CGSize> advances(glyphs.size());
    QVarLengthArray<CGGlyph> cgGlyphs(glyphs.size());

    for (int i = 0; i < glyphs.size() - 1; ++i) {
        advances[i].width = (positions[i + 1].x - positions[i].x).toReal();
        advances[i].height = (positions[i + 1].y - positions[i].y).toReal();
        cgGlyphs[i] = glyphs[i];
    }
    advances[glyphs.size() - 1].width = 0;
    advances[glyphs.size() - 1].height = 0;
    cgGlyphs[glyphs.size() - 1] = glyphs[glyphs.size() - 1];

    CGContextSetFont(ctx, cgFont);
    //NSLog(@"Font inDraw %@  ctfont %@", CGFontCopyFullName(cgFont), CTFontCopyFamilyName(ctfont));

    CGContextSetTextPosition(ctx, positions[0].x.toReal(), positions[0].y.toReal());

    CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, positions[0].x.toReal() + 0.5 * lineThickness().toReal(),
                                 positions[0].y.toReal());

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());
    }

    CGContextSetTextMatrix(ctx, oldTextMatrix);
}

struct ConvertPathInfo
{
    ConvertPathInfo(QPainterPath *newPath, const QPointF &newPos) : path(newPath), pos(newPos) {}
    QPainterPath *path;
    QPointF pos;
};

static void convertCGPathToQPainterPath(void *info, const CGPathElement *element)
{
    ConvertPathInfo *myInfo = static_cast<ConvertPathInfo *>(info);
    switch(element->type) {
        case kCGPathElementMoveToPoint:
            myInfo->path->moveTo(element->points[0].x + myInfo->pos.x(),
                                 element->points[0].y + myInfo->pos.y());
            break;
        case kCGPathElementAddLineToPoint:
            myInfo->path->lineTo(element->points[0].x + myInfo->pos.x(),
                                 element->points[0].y + myInfo->pos.y());
            break;
        case kCGPathElementAddQuadCurveToPoint:
            myInfo->path->quadTo(element->points[0].x + myInfo->pos.x(),
                                 element->points[0].y + myInfo->pos.y(),
                                 element->points[1].x + myInfo->pos.x(),
                                 element->points[1].y + myInfo->pos.y());
            break;
        case kCGPathElementAddCurveToPoint:
            myInfo->path->cubicTo(element->points[0].x + myInfo->pos.x(),
                                  element->points[0].y + myInfo->pos.y(),
                                  element->points[1].x + myInfo->pos.x(),
                                  element->points[1].y + myInfo->pos.y(),
                                  element->points[2].x + myInfo->pos.x(),
                                  element->points[2].y + myInfo->pos.y());
            break;
        case kCGPathElementCloseSubpath:
            myInfo->path->closeSubpath();
            break;
        default:
            qDebug() << "Unhandled path transform type: " << element->type;
    }

}

void QCoreTextFontEngine::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nGlyphs,
                                          QPainterPath *path, QTextItem::RenderFlags)
{

    CGAffineTransform cgMatrix = CGAffineTransformIdentity;
    cgMatrix = CGAffineTransformScale(cgMatrix, 1, -1);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -SYNTHETIC_ITALIC_SKEW, 1, 0, 0));


    for (int i = 0; i < nGlyphs; ++i) {
        QCFType<CGPathRef> cgpath = CTFontCreatePathForGlyph(ctfont, glyphs[i], &cgMatrix);
        ConvertPathInfo info(path, positions[i].toPointF());
        CGPathApply(cgpath, &info, convertCGPathToQPainterPath);
    }
}

QImage QCoreTextFontEngine::imageForGlyph(glyph_t glyph, int margin, bool aa)
{
    const glyph_metrics_t br = boundingBox(glyph);
    QImage im(qRound(br.width)+2, qRound(br.height)+2, QImage::Format_RGB32);
    im.fill(0);

    CGColorSpaceRef colorspace = QCoreGraphicsPaintEngine::macGenericColorSpace();
    uint cgflags = kCGImageAlphaNoneSkipFirst;
#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    cgflags |= kCGBitmapByteOrder32Host;
#endif
    CGContextRef ctx = CGBitmapContextCreate(im.bits(), im.width(), im.height(),
                                             8, im.bytesPerLine(), colorspace,
                                             cgflags);
    CGContextSetFontSize(ctx, fontDef.pixelSize);
    CGContextSetShouldAntialias(ctx, aa ||
                                (fontDef.pointSize > qt_antialiasing_threshold
                                 && !(fontDef.styleStrategy & QFont::NoAntialias)));
    CGContextSetShouldSmoothFonts(ctx, aa);
    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);
    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, 1, 0, 0);

    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, SYNTHETIC_ITALIC_SKEW, 1, 0, 0));

    cgMatrix = CGAffineTransformConcat(cgMatrix, transform);

    CGContextSetTextMatrix(ctx, cgMatrix);
    CGContextSetRGBFillColor(ctx, 1, 1, 1, 1);
    CGContextSetTextDrawingMode(ctx, kCGTextFill);

    ATSFontRef atsfont = CTFontGetPlatformFont(ctfont, 0);
    QCFType<CGFontRef> cgFont = CGFontCreateWithPlatformFont(&atsfont);
    CGContextSetFont(ctx, cgFont);

    qreal pos_x = -br.x.toReal()+1, pos_y = im.height()+br.y.toReal();
    CGContextSetTextPosition(ctx, pos_x, pos_y);

    CGSize advance;
    advance.width = 0;
    advance.height = 0;
    CGGlyph cgGlyph = glyph;
    CGContextShowGlyphsWithAdvances(ctx, &cgGlyph, &advance, 1);

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, pos_x + 0.5 * lineThickness().toReal(), pos_y);
        CGContextShowGlyphsWithAdvances(ctx, &cgGlyph, &advance, 1);
    }

    CGContextRelease(ctx);

    return im;
}

QImage QCoreTextFontEngine::alphaMapForGlyph(glyph_t glyph)
{
    QImage im = imageForGlyph(glyph, 0, false);

    QImage indexed(im.width(), im.height(), QImage::Format_Indexed8);
    QVector<QRgb> colors(256);
    for (int i=0; i<256; ++i)
        colors[i] = qRgba(0, 0, 0, i);
    indexed.setColorTable(colors);

    for (int y=0; y<im.height(); ++y) {
        uint *src = (uint*) im.scanLine(y);
        uchar *dst = indexed.scanLine(y);
        for (int x=0; x<im.width(); ++x) {
            *dst = qGray(*src);
            ++dst;
            ++src;
        }
    }

    return indexed;
}

QImage QCoreTextFontEngine::alphaRGBMapForGlyph(glyph_t glyph, int margin, const QTransform &x)
{
    if (x.type() >= QTransform::TxScale)
        return QFontEngine::alphaRGBMapForGlyph(glyph, margin, x);

    QImage im = imageForGlyph(glyph, margin, true);
    qmacfontengine_gamma_correct(&im);
    return im;
}

void QCoreTextFontEngine::recalcAdvances(int numGlyphs, QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    Q_ASSERT(false);
    Q_UNUSED(numGlyphs);
    Q_UNUSED(glyphs);
    Q_UNUSED(flags);
}

QFontEngine::FaceId QCoreTextFontEngine::faceId() const
{
    return QFontEngine::FaceId();
}

bool QCoreTextFontEngine::canRender(const QChar *string, int len)
{
    QCFType<CTFontRef> retFont = CTFontCreateForString(ctfont,
                  QCFType<CFStringRef>(CFStringCreateWithCharactersNoCopy(0,
                                                              reinterpret_cast<const UniChar *>(string),
                                                                          len, kCFAllocatorNull)),
                          CFRangeMake(0, len));
    return retFont != 0;
    return false;
}

 bool QCoreTextFontEngine::getSfntTableData(uint tag, uchar *buffer, uint *length) const
 {
     QCFType<CFDataRef> table = CTFontCopyTable(ctfont, tag, 0);
     if (!table || !length)
         return false;
     CFIndex tableLength = CFDataGetLength(table);
     int availableLength = *length;
     *length = tableLength;
     if (buffer) {
         if (tableLength > availableLength)
             return false;
         CFDataGetBytes(table, CFRangeMake(0, tableLength), buffer);
     }
     return true;
 }

void QCoreTextFontEngine::getUnscaledGlyph(glyph_t, QPainterPath *, glyph_metrics_t *)
{
    // ###
}

#endif // MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5

#ifndef QT_MAC_USE_COCOA
QFontEngineMacMulti::QFontEngineMacMulti(const ATSFontFamilyRef &atsFamily, const ATSFontRef &atsFontRef, const QFontDef &fontDef, bool kerning)
    : QFontEngineMulti(0)
{
    this->fontDef = fontDef;
    this->kerning = kerning;

    // hopefully (CTFontCreateWithName or CTFontCreateWithFontDescriptor) + CTFontCreateCopyWithSymbolicTraits
    // (or CTFontCreateWithQuickdrawInstance)
    FMFontFamily fmFamily;
    FMFontStyle fntStyle = 0;
    fmFamily = FMGetFontFamilyFromATSFontFamilyRef(atsFamily);
    if (fmFamily == kInvalidFontFamily) {
        // Use the ATSFont then...
        fontID = FMGetFontFromATSFontRef(atsFontRef);
    } else {
        if (fontDef.weight >= QFont::Bold)
            fntStyle |= ::bold;
        if (fontDef.style != QFont::StyleNormal)
            fntStyle |= ::italic;

        FMFontStyle intrinsicStyle;
        FMFont fnt = 0;
        if (FMGetFontFromFontFamilyInstance(fmFamily, fntStyle, &fnt, &intrinsicStyle) == noErr)
           fontID = FMGetATSFontRefFromFont(fnt);
    }

    // CFDictionaryRef, <CTStringAttributes.h>
    OSStatus status;

    status = ATSUCreateTextLayout(&textLayout);
    Q_ASSERT(status == noErr);

    const int maxAttributeCount = 5;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    Fixed size = FixRatio(fontDef.pixelSize, 1);
    tags[attributeCount] = kATSUSizeTag;
    sizes[attributeCount] = sizeof(size);
    values[attributeCount] = &size;
    ++attributeCount;

    tags[attributeCount] = kATSUFontTag;
    sizes[attributeCount] = sizeof(fontID);
    values[attributeCount] = &this->fontID;
    ++attributeCount;

    transform = CGAffineTransformIdentity;
    if (fontDef.stretch != 100) {
        transform = CGAffineTransformMakeScale(float(fontDef.stretch) / float(100), 1);
        tags[attributeCount] = kATSUFontMatrixTag;
        sizes[attributeCount] = sizeof(transform);
        values[attributeCount] = &transform;
        ++attributeCount;
    }

    status = ATSUCreateStyle(&style);
    Q_ASSERT(status == noErr);

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    status = ATSUSetAttributes(style, attributeCount, tags, sizes, values);
    Q_ASSERT(status == noErr);

    QFontEngineMac *fe = new QFontEngineMac(style, fontID, fontDef, this);
    fe->ref.ref();
    engines.append(fe);
}

QFontEngineMacMulti::~QFontEngineMacMulti()
{
    ATSUDisposeTextLayout(textLayout);
    ATSUDisposeStyle(style);

    for (int i = 0; i < engines.count(); ++i) {
        QFontEngineMac *fe = const_cast<QFontEngineMac *>(static_cast<const QFontEngineMac *>(engines.at(i)));
        fe->multiEngine = 0;
        if (!fe->ref.deref())
            delete fe;
    }
    engines.clear();
}

struct QGlyphLayoutInfo
{
    QGlyphLayout *glyphs;
    int *numGlyphs;
    bool callbackCalled;
    int *mappedFonts;
    QTextEngine::ShaperFlags flags;
    QFontEngineMacMulti::ShaperItem *shaperItem;
    unsigned int styleStrategy;
};

static OSStatus atsuPostLayoutCallback(ATSULayoutOperationSelector selector, ATSULineRef lineRef, URefCon refCon,
                                 void *operationExtraParameter, ATSULayoutOperationCallbackStatus *callbackStatus)
{
    Q_UNUSED(selector);
    Q_UNUSED(operationExtraParameter);

    QGlyphLayoutInfo *nfo = reinterpret_cast<QGlyphLayoutInfo *>(refCon);
    nfo->callbackCalled = true;

    ATSLayoutRecord *layoutData = 0;
    ItemCount itemCount = 0;

    OSStatus e = noErr;
    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                                   /*iCreate =*/ false,
                                                   (void **) &layoutData,
                                                   &itemCount);
    if (e != noErr)
        return e;

    *nfo->numGlyphs = itemCount - 1;

    Fixed *baselineDeltas = 0;

    e = ATSUDirectGetLayoutDataArrayPtrFromLineRef(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                                   /*iCreate =*/ true,
                                                   (void **) &baselineDeltas,
                                                   &itemCount);
    if (e != noErr)
        return e;

    int nextCharStop = -1;
    int currentClusterGlyph = -1; // first glyph in log cluster
    QFontEngineMacMulti::ShaperItem *item = nfo->shaperItem;
    if (item->charAttributes) {
        item = nfo->shaperItem;
#if !defined(QT_NO_DEBUG)
        int surrogates = 0;
        const QChar *str = item->string;
        for (int i = item->from; i < item->from + item->length - 1; ++i) {
            surrogates += (str[i].unicode() >= 0xd800 && str[i].unicode() < 0xdc00
                           && str[i+1].unicode() >= 0xdc00 && str[i+1].unicode() < 0xe000);
        }
#endif
        for (nextCharStop = item->from; nextCharStop < item->from + item->length; ++nextCharStop)
            if (item->charAttributes[nextCharStop].charStop)
                break;
        nextCharStop -= item->from;
    }

    nfo->glyphs->attributes[0].clusterStart = true;
    int glyphIdx = 0;
    int glyphIncrement = 1;
    if (nfo->flags & QTextEngine::RightToLeft) {
        glyphIdx  = itemCount - 2;
        glyphIncrement = -1;
    }
    for (int i = 0; i < *nfo->numGlyphs; ++i, glyphIdx += glyphIncrement) {

        int charOffset = layoutData[glyphIdx].originalOffset / sizeof(UniChar);
        const int fontIdx = nfo->mappedFonts[charOffset];

        ATSGlyphRef glyphId = layoutData[glyphIdx].glyphID;

        QFixed yAdvance = FixedToQFixed(baselineDeltas[glyphIdx]);
        QFixed xAdvance = FixedToQFixed(layoutData[glyphIdx + 1].realPos - layoutData[glyphIdx].realPos);

        if (nfo->styleStrategy & QFont::ForceIntegerMetrics) {
            yAdvance = yAdvance.round();
            xAdvance = xAdvance.round();
        }

        if (glyphId != 0xffff || i == 0) {
            if (i < nfo->glyphs->numGlyphs)
            {
                nfo->glyphs->glyphs[i] = (glyphId & 0x00ffffff) | (fontIdx << 24);

                nfo->glyphs->advances_y[i] = yAdvance;
                nfo->glyphs->advances_x[i] = xAdvance;
            }
        } else {
            // ATSUI gives us 0xffff as glyph id at the index in the glyph array for
            // a character position that maps to a ligtature. Such a glyph id does not
            // result in any visual glyph, but it may have an advance, which is why we
            // sum up the glyph advances.
            --i;
            nfo->glyphs->advances_y[i] += yAdvance;
            nfo->glyphs->advances_x[i] += xAdvance;
            *nfo->numGlyphs -= 1;
        }

        if (item->log_clusters) {
            if (charOffset >= nextCharStop) {
                nfo->glyphs->attributes[i].clusterStart = true;
                currentClusterGlyph = i;

                ++nextCharStop;
                for (; nextCharStop < item->length; ++nextCharStop)
                    if (item->charAttributes[item->from + nextCharStop].charStop)
                        break;
            } else {
                if (currentClusterGlyph == -1)
                    currentClusterGlyph = i;
            }
            item->log_clusters[charOffset] = currentClusterGlyph;

            // surrogate handling
            if (charOffset < item->length - 1) {
                QChar current = item->string[item->from + charOffset];
                QChar next = item->string[item->from + charOffset + 1];
                if (current.unicode() >= 0xd800 && current.unicode() < 0xdc00
                    && next.unicode() >= 0xdc00 && next.unicode() < 0xe000) {
                    item->log_clusters[charOffset + 1] = currentClusterGlyph;
                }
            }
        }
    }

    /*
    if (item) {
        qDebug() << "resulting logclusters:";
        for (int i = 0; i < item->length; ++i)
            qDebug() << "logClusters[" << i << "] =" << item->log_clusters[i];
        qDebug() << "clusterstarts:";
        for (int i = 0; i < *nfo->numGlyphs; ++i)
            qDebug() << "clusterStart[" << i << "] =" << nfo->glyphs[i].attributes.clusterStart;
    }
    */

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataBaselineDeltaFixedArray,
                                        (void **) &baselineDeltas);

    ATSUDirectReleaseLayoutDataArrayPtr(lineRef, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent,
                                        (void **) &layoutData);

    *callbackStatus = kATSULayoutOperationCallbackStatusHandled;
    return noErr;
}

int QFontEngineMacMulti::fontIndexForFontID(ATSUFontID id) const
{
    for (int i = 0; i < engines.count(); ++i) {
        if (engineAt(i)->fontID == id)
            return i;
    }

    QFontEngineMacMulti *that = const_cast<QFontEngineMacMulti *>(this);
    QFontEngineMac *fe = new QFontEngineMac(style, id, fontDef, that);
    fe->ref.ref();
    that->engines.append(fe);
    return engines.count() - 1;
}

bool QFontEngineMacMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    return stringToCMap(str, len, glyphs, nglyphs, flags, /*logClusters=*/0, /*charAttributes=*/0);
}

bool QFontEngineMacMulti::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,
                                       unsigned short *logClusters, const HB_CharAttributes *charAttributes) const
{
    if (*nglyphs < len) {
        *nglyphs = len;
        return false;
    }

    ShaperItem shaperItem;
    shaperItem.string = str;
    shaperItem.from = 0;
    shaperItem.length = len;
    shaperItem.glyphs = *glyphs;
    shaperItem.glyphs.numGlyphs = *nglyphs;
    shaperItem.flags = flags;
    shaperItem.log_clusters = logClusters;
    shaperItem.charAttributes = charAttributes;

    const int maxChars = qMax(1,
                              int(SHRT_MAX / maxCharWidth())
                              - 10 // subtract a few to be on the safe side
                             );
    if (len < maxChars || !charAttributes)
        return stringToCMapInternal(str, len, glyphs, nglyphs, flags, &shaperItem);

    int charIdx = 0;
    int glyphIdx = 0;
    ShaperItem tmpItem = shaperItem;

    do {
        tmpItem.from = shaperItem.from + charIdx;

        int charCount = qMin(maxChars, len - charIdx);

        int lastWhitespace = tmpItem.from + charCount - 1;
        int lastSoftBreak = lastWhitespace;
        int lastCharStop = lastSoftBreak;
        for (int i = lastCharStop; i >= tmpItem.from; --i) {
            if (tmpItem.charAttributes[i].whiteSpace) {
                lastWhitespace = i;
                break;
            } if (tmpItem.charAttributes[i].lineBreakType != HB_NoBreak) {
                lastSoftBreak = i;
            } if (tmpItem.charAttributes[i].charStop) {
                lastCharStop = i;
            }
        }
        charCount = qMin(lastWhitespace, qMin(lastSoftBreak, lastCharStop)) - tmpItem.from + 1;

        int glyphCount = shaperItem.glyphs.numGlyphs - glyphIdx;
        if (glyphCount <= 0)
            return false;
        tmpItem.length = charCount;
        tmpItem.glyphs = shaperItem.glyphs.mid(glyphIdx, glyphCount);
        tmpItem.log_clusters = shaperItem.log_clusters + charIdx;
        if (!stringToCMapInternal(tmpItem.string + tmpItem.from, tmpItem.length,
                                  &tmpItem.glyphs, &glyphCount, flags,
                                  &tmpItem)) {
            *nglyphs = glyphIdx + glyphCount;
            return false;
	}
        for (int i = 0; i < charCount; ++i)
            tmpItem.log_clusters[i] += glyphIdx;
        glyphIdx += glyphCount;
        charIdx += charCount;
    } while (charIdx < len);
    *nglyphs = glyphIdx;
    glyphs->numGlyphs = glyphIdx;

    return true;
}

bool QFontEngineMacMulti::stringToCMapInternal(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags,ShaperItem *shaperItem) const
{
    //qDebug() << "stringToCMap" << QString(str, len);

    OSStatus e = noErr;

    e = ATSUSetTextPointerLocation(textLayout, (UniChar *)(str), 0, len, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextPointerLocation %s: %d", long(e), __FILE__, __LINE__);
        return false;
    }

    QGlyphLayoutInfo nfo;
    nfo.glyphs = glyphs;
    nfo.numGlyphs = nglyphs;
    nfo.callbackCalled = false;
    nfo.flags = flags;
    nfo.shaperItem = shaperItem;
    nfo.styleStrategy = fontDef.styleStrategy;

    int prevNumGlyphs = *nglyphs;

    QVarLengthArray<int> mappedFonts(len);
    for (int i = 0; i < len; ++i)
        mappedFonts[i] = 0;
    nfo.mappedFonts = mappedFonts.data();

    Q_ASSERT(sizeof(void *) <= sizeof(URefCon));
    e = ATSUSetTextLayoutRefCon(textLayout, (URefCon)&nfo);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetTextLayoutRefCon %s: %d", long(e), __FILE__, __LINE__);
        return false;
    }

    {
        const int maxAttributeCount = 3;
        ATSUAttributeTag tags[maxAttributeCount + 1];
        ByteCount sizes[maxAttributeCount + 1];
        ATSUAttributeValuePtr values[maxAttributeCount + 1];
        int attributeCount = 0;

        tags[attributeCount] = kATSULineLayoutOptionsTag;
        ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment
                                       | kATSLineIgnoreFontLeading
                                       | kATSLineNoSpecialJustification // we do kashidas ourselves
                                       | kATSLineDisableAllJustification
                                       ;

        if (fontDef.styleStrategy & QFont::NoAntialias)
            layopts |= kATSLineNoAntiAliasing;

        if (!kerning)
            layopts |= kATSLineDisableAllKerningAdjustments;

        values[attributeCount] = &layopts;
        sizes[attributeCount] = sizeof(layopts);
        ++attributeCount;

        tags[attributeCount] = kATSULayoutOperationOverrideTag;
        ATSULayoutOperationOverrideSpecifier spec;
        spec.operationSelector = kATSULayoutOperationPostLayoutAdjustment;
        spec.overrideUPP = atsuPostLayoutCallback;
        values[attributeCount] = &spec;
        sizes[attributeCount] = sizeof(spec);
        ++attributeCount;

        // CTWritingDirection
        Boolean direction;
        if (flags & QTextEngine::RightToLeft)
            direction = kATSURightToLeftBaseDirection;
        else
            direction = kATSULeftToRightBaseDirection;
        tags[attributeCount] = kATSULineDirectionTag;
        values[attributeCount] = &direction;
        sizes[attributeCount] = sizeof(direction);
        ++attributeCount;

        Q_ASSERT(attributeCount < maxAttributeCount + 1);
        e = ATSUSetLayoutControls(textLayout, attributeCount, tags, sizes, values);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUSetLayoutControls %s: %d", long(e), __FILE__, __LINE__);
            return false;
        }

    }

    e = ATSUSetRunStyle(textLayout, style, 0, len);
    if (e != noErr) {
        qWarning("Qt: internal: %ld: Error ATSUSetRunStyle %s: %d", long(e), __FILE__, __LINE__);
        return false;
    }

    if (!(fontDef.styleStrategy & QFont::NoFontMerging)) {
        int pos = 0;
        do {
            ATSUFontID substFont = 0;
            UniCharArrayOffset changedOffset = 0;
            UniCharCount changeCount = 0;

            e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                     &substFont, &changedOffset,
                                     &changeCount);
            if (e == kATSUFontsMatched) {
                int fontIdx = fontIndexForFontID(substFont);
                for (uint i = 0; i < changeCount; ++i)
                    mappedFonts[changedOffset + i] = fontIdx;
                pos = changedOffset + changeCount;
                ATSUSetRunStyle(textLayout, engineAt(fontIdx)->style, changedOffset, changeCount);
            } else if (e == kATSUFontsNotMatched) {
                pos = changedOffset + changeCount;
            }
        } while (pos < len && e != noErr);
    }
    {    // trigger the a layout
        // CFAttributedStringCreate, CTFramesetterCreateWithAttributedString (or perhaps Typesetter)
        Rect rect;
        e = ATSUMeasureTextImage(textLayout, kATSUFromTextBeginning, kATSUToTextEnd,
                                 /*iLocationX =*/ 0, /*iLocationY =*/ 0,
                                 &rect);
        if (e != noErr) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage %s: %d", long(e), __FILE__, __LINE__);
            return false;
        }
    }

    if (!nfo.callbackCalled) {
            qWarning("Qt: internal: %ld: Error ATSUMeasureTextImage did not trigger callback %s: %d", long(e), __FILE__, __LINE__);
            return false;
    }

    ATSUClearLayoutCache(textLayout, kATSUFromTextBeginning);
    if (prevNumGlyphs < *nfo.numGlyphs)
        return false;
    return true;
}

void QFontEngineMacMulti::recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    Q_ASSERT(false);
    Q_UNUSED(glyphs);
    Q_UNUSED(flags);
}

void QFontEngineMacMulti::doKerning(QGlyphLayout *, QTextEngine::ShaperFlags) const
{
    //Q_ASSERT(false);
}

void QFontEngineMacMulti::loadEngine(int /*at*/)
{
    // should never be called!
    Q_ASSERT(false);
}

bool QFontEngineMacMulti::canRender(const QChar *string, int len)
{
    ATSUSetTextPointerLocation(textLayout, reinterpret_cast<const UniChar *>(string), 0, len, len);
    ATSUSetRunStyle(textLayout, style, 0, len);

    OSStatus e = noErr;
    int pos = 0;
    do {
        FMFont substFont = 0;
        UniCharArrayOffset changedOffset = 0;
        UniCharCount changeCount = 0;

        // CTFontCreateForString
        e = ATSUMatchFontsToText(textLayout, pos, len - pos,
                                 &substFont, &changedOffset,
                                 &changeCount);
        if (e == kATSUFontsMatched) {
            pos = changedOffset + changeCount;
        } else if (e == kATSUFontsNotMatched) {
            break;
        }
    } while (pos < len && e != noErr);

    return e == noErr || e == kATSUFontsMatched;
}

QFontEngineMac::QFontEngineMac(ATSUStyle baseStyle, ATSUFontID fontID, const QFontDef &def, QFontEngineMacMulti *multiEngine)
    : fontID(fontID), multiEngine(multiEngine), cmap(0), symbolCMap(false)
{
    fontDef = def;
    ATSUCreateAndCopyStyle(baseStyle, &style);
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fontID);
    cgFont = CGFontCreateWithPlatformFont(&atsFont);

    const int maxAttributeCount = 4;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    synthesisFlags = 0;

    // synthesizing using CG is not recommended
    quint16 macStyle = 0;
    {
        uchar data[4];
        ByteCount len = 4;
        if (ATSFontGetTable(atsFont, MAKE_TAG('h', 'e', 'a', 'd'), 44, 4, &data, &len) == noErr)
            macStyle = qFromBigEndian<quint16>(data);
    }

    Boolean atsuBold = false;
    Boolean atsuItalic = false;
    if (fontDef.weight >= QFont::Bold) {
        if (!(macStyle & 1)) {
            synthesisFlags |= SynthesizedBold;
            atsuBold = true;
            tags[attributeCount] = kATSUQDBoldfaceTag;
            sizes[attributeCount] = sizeof(atsuBold);
            values[attributeCount] = &atsuBold;
            ++attributeCount;
        }
    }
    if (fontDef.style != QFont::StyleNormal) {
        if (!(macStyle & 2)) {
            synthesisFlags |= SynthesizedItalic;
            atsuItalic = true;
            tags[attributeCount] = kATSUQDItalicTag;
            sizes[attributeCount] = sizeof(atsuItalic);
            values[attributeCount] = &atsuItalic;
            ++attributeCount;
        }
    }

    tags[attributeCount] = kATSUFontTag;
    values[attributeCount] = &fontID;
    sizes[attributeCount] = sizeof(fontID);
    ++attributeCount;

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    OSStatus err = ATSUSetAttributes(style, attributeCount, tags, sizes, values);
    Q_ASSERT(err == noErr);
    Q_UNUSED(err);

    // CTFontCopyTable
    quint16 tmpFsType;
    if (ATSFontGetTable(atsFont, MAKE_TAG('O', 'S', '/', '2'), 8, 2, &tmpFsType, 0) == noErr)
       fsType = qFromBigEndian<quint16>(tmpFsType);
    else
        fsType = 0;

    if (multiEngine)
	transform = multiEngine->transform;
    else
	transform = CGAffineTransformIdentity;

    ATSUTextMeasurement metric;

    ATSUGetAttribute(style, kATSUAscentTag, sizeof(metric), &metric, 0);
    m_ascent = FixRound(metric);

    ATSUGetAttribute(style, kATSUDescentTag, sizeof(metric), &metric, 0);
    m_descent = FixRound(metric);

    ATSUGetAttribute(style, kATSULeadingTag, sizeof(metric), &metric, 0);
    m_leading = FixRound(metric);

    ATSFontMetrics metrics;

    ATSFontGetHorizontalMetrics(FMGetATSFontRefFromFont(fontID), kATSOptionFlagsDefault, &metrics);
    m_maxCharWidth = metrics.maxAdvanceWidth * fontDef.pointSize;

    ATSFontGetHorizontalMetrics(FMGetATSFontRefFromFont(fontID), kATSOptionFlagsDefault, &metrics);
    m_xHeight = QFixed::fromReal(metrics.xHeight * fontDef.pointSize);

    ATSFontGetHorizontalMetrics(FMGetATSFontRefFromFont(fontID), kATSOptionFlagsDefault, &metrics);
    m_averageCharWidth = QFixed::fromReal(metrics.avgAdvanceWidth * fontDef.pointSize);

    // Use width of 'X' if ATSFontGetHorizontalMetrics returns 0 for avgAdvanceWidth.
    if (m_averageCharWidth == QFixed(0)) {
        QChar c('X');
        QGlyphLayoutArray<1> glyphs;
        int nglyphs = 1;
        stringToCMap(&c, 1, &glyphs, &nglyphs, 0);
        glyph_metrics_t metrics = boundingBox(glyphs);
        m_averageCharWidth =  metrics.width;
    }
}

QFontEngineMac::~QFontEngineMac()
{
    ATSUDisposeStyle(style);
}

static inline unsigned int getChar(const QChar *str, int &i, const int len)
{
    unsigned int uc = str[i].unicode();
    if (uc >= 0xd800 && uc < 0xdc00 && i < len-1) {
        uint low = str[i+1].unicode();
       if (low >= 0xdc00 && low < 0xe000) {
            uc = (uc - 0xd800)*0x400 + (low - 0xdc00) + 0x10000;
            ++i;
        }
    }
    return uc;
}

bool QFontEngineMac::stringToCMap(const QChar *str, int len, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const
{
    if (!cmap) {
        cmapTable = getSfntTable(MAKE_TAG('c', 'm', 'a', 'p'));
        int size = 0;
        cmap = getCMap(reinterpret_cast<const uchar *>(cmapTable.constData()), cmapTable.size(), &symbolCMap, &size);
        if (!cmap)
            return false;
    }
    if (symbolCMap) {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            glyphs->glyphs[i] = getTrueTypeGlyphIndex(cmap, uc);
            if(!glyphs->glyphs[i] && uc < 0x100)
                glyphs->glyphs[i] = getTrueTypeGlyphIndex(cmap, uc + 0xf000);
        }
    } else {
        for (int i = 0; i < len; ++i) {
            unsigned int uc = getChar(str, i, len);
            glyphs->glyphs[i] = getTrueTypeGlyphIndex(cmap, uc);
        }
    }

    *nglyphs = len;
    glyphs->numGlyphs = *nglyphs;

    if (!(flags & QTextEngine::GlyphIndicesOnly))
        recalcAdvances(glyphs, flags);

    return true;
}

void QFontEngineMac::recalcAdvances(QGlyphLayout *glyphs, QTextEngine::ShaperFlags flags) const
{
    Q_UNUSED(flags)

    QVarLengthArray<GlyphID> atsuGlyphs(glyphs->numGlyphs);
    for (int i = 0; i < glyphs->numGlyphs; ++i)
        atsuGlyphs[i] = glyphs->glyphs[i];

    QVarLengthArray<ATSGlyphScreenMetrics> metrics(glyphs->numGlyphs);

    ATSUGlyphGetScreenMetrics(style, glyphs->numGlyphs, atsuGlyphs.data(), sizeof(GlyphID),
                              /* iForcingAntiAlias =*/ false,
                              /* iAntiAliasSwitch =*/true,
                              metrics.data());

    for (int i = 0; i < glyphs->numGlyphs; ++i) {
        glyphs->advances_x[i] = QFixed::fromReal(metrics[i].deviceAdvance.x);
        glyphs->advances_y[i] = QFixed::fromReal(metrics[i].deviceAdvance.y);

        if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
            glyphs->advances_x[i] = glyphs->advances_x[i].round();
            glyphs->advances_y[i] = glyphs->advances_y[i].round();
        }
    }
}

glyph_metrics_t QFontEngineMac::boundingBox(const QGlyphLayout &glyphs)
{
    QFixed w;
    bool round = fontDef.styleStrategy & QFont::ForceIntegerMetrics;
    for (int i = 0; i < glyphs.numGlyphs; ++i) {
        w += round ? glyphs.effectiveAdvance(i).round()
                   : glyphs.effectiveAdvance(i);
    }
    return glyph_metrics_t(0, -(ascent()), w - lastRightBearing(glyphs, round), ascent()+descent(), w, 0);
}

glyph_metrics_t QFontEngineMac::boundingBox(glyph_t glyph)
{
    GlyphID atsuGlyph = glyph;

    ATSGlyphScreenMetrics metrics;

    ATSUGlyphGetScreenMetrics(style, 1, &atsuGlyph, 0,
                              /* iForcingAntiAlias =*/ false,
                              /* iAntiAliasSwitch =*/true,
                              &metrics);

    // ### check again

    glyph_metrics_t gm;
    gm.width = int(metrics.width);
    gm.height = int(metrics.height);
    gm.x = QFixed::fromReal(metrics.topLeft.x);
    gm.y = -QFixed::fromReal(metrics.topLeft.y);
    gm.xoff = QFixed::fromReal(metrics.deviceAdvance.x);
    gm.yoff = QFixed::fromReal(metrics.deviceAdvance.y);

    if (fontDef.styleStrategy & QFont::ForceIntegerMetrics) {
        gm.x = gm.x.floor();
        gm.y = gm.y.floor();
        gm.xoff = gm.xoff.round();
        gm.yoff = gm.yoff.round();
    }

    return gm;
}

QFixed QFontEngineMac::ascent() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? m_ascent.round()
            : m_ascent;
}

QFixed QFontEngineMac::descent() const
{
    // subtract a pixel to even out the historical +1 in QFontMetrics::height().
    // Fix in Qt 5.
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? m_descent.round() - 1
            : m_descent;
}

QFixed QFontEngineMac::leading() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? m_leading.round()
            : m_leading;
}

qreal QFontEngineMac::maxCharWidth() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? qRound(m_maxCharWidth)
            : m_maxCharWidth;
}

QFixed QFontEngineMac::xHeight() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? m_xHeight.round()
            : m_xHeight;
}

QFixed QFontEngineMac::averageCharWidth() const
{
    return (fontDef.styleStrategy & QFont::ForceIntegerMetrics)
            ? m_averageCharWidth.round()
            : m_averageCharWidth;
}

static void addGlyphsToPathHelper(ATSUStyle style, glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path)
{
    if (!numGlyphs)
        return;

    OSStatus e;

    QMacFontPath fontpath(0, 0, path);
    ATSCubicMoveToUPP moveTo = NewATSCubicMoveToUPP(QMacFontPath::moveTo);
    ATSCubicLineToUPP lineTo = NewATSCubicLineToUPP(QMacFontPath::lineTo);
    ATSCubicCurveToUPP cubicTo = NewATSCubicCurveToUPP(QMacFontPath::cubicTo);
    ATSCubicClosePathUPP closePath = NewATSCubicClosePathUPP(QMacFontPath::closePath);

    // CTFontCreatePathForGlyph
    for (int i = 0; i < numGlyphs; ++i) {
        GlyphID glyph = glyphs[i];

        fontpath.setPosition(positions[i].x.toReal(), positions[i].y.toReal());
        ATSUGlyphGetCubicPaths(style, glyph, moveTo, lineTo,
                               cubicTo, closePath, &fontpath, &e);
    }

    DisposeATSCubicMoveToUPP(moveTo);
    DisposeATSCubicLineToUPP(lineTo);
    DisposeATSCubicCurveToUPP(cubicTo);
    DisposeATSCubicClosePathUPP(closePath);
}

void QFontEngineMac::addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int numGlyphs, QPainterPath *path,
                                           QTextItem::RenderFlags)
{
    addGlyphsToPathHelper(style, glyphs, positions, numGlyphs, path);
}


/*!
  Helper function for alphaMapForGlyph and alphaRGBMapForGlyph. The two are identical, except for
  the subpixel antialiasing...
*/
QImage QFontEngineMac::imageForGlyph(glyph_t glyph, int margin, bool colorful)
{
    const glyph_metrics_t br = boundingBox(glyph);
    QImage im(qRound(br.width)+2, qRound(br.height)+4, QImage::Format_RGB32);
    im.fill(0xff000000);

    CGColorSpaceRef colorspace = QCoreGraphicsPaintEngine::macGenericColorSpace();
    uint cgflags = kCGImageAlphaNoneSkipFirst;
#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    cgflags |= kCGBitmapByteOrder32Host;
#endif
    CGContextRef ctx = CGBitmapContextCreate(im.bits(), im.width(), im.height(),
                                             8, im.bytesPerLine(), colorspace,
                                             cgflags);
    CGContextSetFontSize(ctx, fontDef.pixelSize);
    CGContextSetShouldAntialias(ctx, fontDef.pointSize > qt_antialiasing_threshold && !(fontDef.styleStrategy & QFont::NoAntialias));
    // turn off sub-pixel hinting - no support for that in OpenGL
    CGContextSetShouldSmoothFonts(ctx, colorful);
    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);
    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, 1, 0, 0);
    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, SYNTHETIC_ITALIC_SKEW, 1, 0, 0));

    cgMatrix = CGAffineTransformConcat(cgMatrix, transform);

    CGContextSetTextMatrix(ctx, cgMatrix);
    CGContextSetRGBFillColor(ctx, 1, 1, 1, 1);
    CGContextSetTextDrawingMode(ctx, kCGTextFill);
    CGContextSetFont(ctx, cgFont);

    qreal pos_x = -br.x.toReal() + 1;
    qreal pos_y = im.height() + br.y.toReal() - 2;
    CGContextSetTextPosition(ctx, pos_x, pos_y);

    CGSize advance;
    advance.width = 0;
    advance.height = 0;
    CGGlyph cgGlyph = glyph;
    CGContextShowGlyphsWithAdvances(ctx, &cgGlyph, &advance, 1);

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, pos_x + 0.5 * lineThickness().toReal(), pos_y);
        CGContextShowGlyphsWithAdvances(ctx, &cgGlyph, &advance, 1);
    }

    CGContextRelease(ctx);

    return im;
}

QImage QFontEngineMac::alphaMapForGlyph(glyph_t glyph)
{
    QImage im = imageForGlyph(glyph, 2, false);

    QImage indexed(im.width(), im.height(), QImage::Format_Indexed8);
    QVector<QRgb> colors(256);
    for (int i=0; i<256; ++i)
        colors[i] = qRgba(0, 0, 0, i);
    indexed.setColorTable(colors);

    for (int y=0; y<im.height(); ++y) {
        uint *src = (uint*) im.scanLine(y);
        uchar *dst = indexed.scanLine(y);
        for (int x=0; x<im.width(); ++x) {
            *dst = qGray(*src);
            ++dst;
            ++src;
        }
    }

    return indexed;
}

QImage QFontEngineMac::alphaRGBMapForGlyph(glyph_t glyph, int margin, const QTransform &t)
{
    QImage im = imageForGlyph(glyph, margin, true);

    if (t.type() >= QTransform::TxScale) {
        im = im.transformed(t);
    }

    qmacfontengine_gamma_correct(&im);

    return im;
}


bool QFontEngineMac::canRender(const QChar *string, int len)
{
    Q_ASSERT(false);
    Q_UNUSED(string);
    Q_UNUSED(len);
    return false;
}

void QFontEngineMac::draw(CGContextRef ctx, qreal x, qreal y, const QTextItemInt &ti, int paintDeviceHeight)
{
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix;
    matrix.translate(x, y);
    getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);
    if (glyphs.size() == 0)
        return;

    CGContextSetFontSize(ctx, fontDef.pixelSize);

    CGAffineTransform oldTextMatrix = CGContextGetTextMatrix(ctx);

    CGAffineTransform cgMatrix = CGAffineTransformMake(1, 0, 0, -1, 0, -paintDeviceHeight);

    CGAffineTransformConcat(cgMatrix, oldTextMatrix);

    if (synthesisFlags & QFontEngine::SynthesizedItalic)
        cgMatrix = CGAffineTransformConcat(cgMatrix, CGAffineTransformMake(1, 0, -SYNTHETIC_ITALIC_SKEW, 1, 0, 0));

    cgMatrix = CGAffineTransformConcat(cgMatrix, transform);

    CGContextSetTextMatrix(ctx, cgMatrix);

    CGContextSetTextDrawingMode(ctx, kCGTextFill);


    QVarLengthArray<CGSize> advances(glyphs.size());
    QVarLengthArray<CGGlyph> cgGlyphs(glyphs.size());

    for (int i = 0; i < glyphs.size() - 1; ++i) {
        advances[i].width = (positions[i + 1].x - positions[i].x).toReal();
        advances[i].height = (positions[i + 1].y - positions[i].y).toReal();
        cgGlyphs[i] = glyphs[i];
    }
    advances[glyphs.size() - 1].width = 0;
    advances[glyphs.size() - 1].height = 0;
    cgGlyphs[glyphs.size() - 1] = glyphs[glyphs.size() - 1];

    CGContextSetFont(ctx, cgFont);

    CGContextSetTextPosition(ctx, positions[0].x.toReal(), positions[0].y.toReal());

    CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());

    if (synthesisFlags & QFontEngine::SynthesizedBold) {
        CGContextSetTextPosition(ctx, positions[0].x.toReal() + 0.5 * lineThickness().toReal(),
                                      positions[0].y.toReal());

        CGContextShowGlyphsWithAdvances(ctx, cgGlyphs.data(), advances.data(), glyphs.size());
    }

    CGContextSetTextMatrix(ctx, oldTextMatrix);
}

QFontEngine::FaceId QFontEngineMac::faceId() const
{
    FaceId ret;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_5) {
    // CTFontGetPlatformFont
    FSRef ref;
    if (ATSFontGetFileReference(FMGetATSFontRefFromFont(fontID), &ref) != noErr)
        return ret;
    ret.filename = QByteArray(128, 0);
    ret.index = fontID;
    FSRefMakePath(&ref, (UInt8 *)ret.filename.data(), ret.filename.size());
}else
#endif
{
    FSSpec spec;
    if (ATSFontGetFileSpecification(FMGetATSFontRefFromFont(fontID), &spec) != noErr)
        return ret;

    FSRef ref;
    FSpMakeFSRef(&spec, &ref);
    ret.filename = QByteArray(128, 0);
    ret.index = fontID;
    FSRefMakePath(&ref, (UInt8 *)ret.filename.data(), ret.filename.size());
}
    return ret;
}

QByteArray QFontEngineMac::getSfntTable(uint tag) const
{
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fontID);

    ByteCount length;
    OSStatus status = ATSFontGetTable(atsFont, tag, 0, 0, 0, &length);
    if (status != noErr)
        return QByteArray();
    QByteArray table(length, 0);
    // CTFontCopyTable
    status = ATSFontGetTable(atsFont, tag, 0, table.length(), table.data(), &length);
    if (status != noErr)
        return QByteArray();
    return table;
}

QFontEngine::Properties QFontEngineMac::properties() const
{
    QFontEngine::Properties props;
    ATSFontRef atsFont = FMGetATSFontRefFromFont(fontID);
    quint16 tmp;
    // CTFontGetUnitsPerEm
    if (ATSFontGetTable(atsFont, MAKE_TAG('h', 'e', 'a', 'd'), 18, 2, &tmp, 0) == noErr)
       props.emSquare = qFromBigEndian<quint16>(tmp);
    struct {
        qint16 xMin;
        qint16 yMin;
        qint16 xMax;
        qint16 yMax;
    } bbox;
    bbox.xMin = bbox.xMax = bbox.yMin = bbox.yMax = 0;
    // CTFontGetBoundingBox
    if (ATSFontGetTable(atsFont, MAKE_TAG('h', 'e', 'a', 'd'), 36, 8, &bbox, 0) == noErr) {
        bbox.xMin = qFromBigEndian<quint16>(bbox.xMin);
        bbox.yMin = qFromBigEndian<quint16>(bbox.yMin);
        bbox.xMax = qFromBigEndian<quint16>(bbox.xMax);
        bbox.yMax = qFromBigEndian<quint16>(bbox.yMax);
    }
    struct {
        qint16 ascender;
        qint16 descender;
        qint16 linegap;
    } metrics;
    metrics.ascender = metrics.descender = metrics.linegap = 0;
    // CTFontGetAscent, etc.
    if (ATSFontGetTable(atsFont, MAKE_TAG('h', 'h', 'e', 'a'), 4, 6, &metrics, 0) == noErr) {
        metrics.ascender = qFromBigEndian<quint16>(metrics.ascender);
        metrics.descender = qFromBigEndian<quint16>(metrics.descender);
        metrics.linegap = qFromBigEndian<quint16>(metrics.linegap);
    }
    props.ascent = metrics.ascender;
    props.descent = -metrics.descender;
    props.leading = metrics.linegap;
    props.boundingBox = QRectF(bbox.xMin, -bbox.yMax,
                           bbox.xMax - bbox.xMin,
                           bbox.yMax - bbox.yMin);
    props.italicAngle = 0;
    props.capHeight = props.ascent;

    qint16 lw = 0;
    // fonts lie
    if (ATSFontGetTable(atsFont, MAKE_TAG('p', 'o', 's', 't'), 10, 2, &lw, 0) == noErr)
       lw = qFromBigEndian<quint16>(lw);
    props.lineWidth = lw;

    // CTFontCopyPostScriptName
    QCFString psName;
    if (ATSFontGetPostScriptName(FMGetATSFontRefFromFont(fontID), kATSOptionFlagsDefault, &psName) == noErr)
        props.postscriptName = QString(psName).toUtf8();
    props.postscriptName = QFontEngine::convertToPostscriptFontFamilyName(props.postscriptName);
    return props;
}

void QFontEngineMac::getUnscaledGlyph(glyph_t glyph, QPainterPath *path, glyph_metrics_t *metrics)
{
    ATSUStyle unscaledStyle;
    ATSUCreateAndCopyStyle(style, &unscaledStyle);

    int emSquare = properties().emSquare.toInt();

    const int maxAttributeCount = 4;
    ATSUAttributeTag tags[maxAttributeCount + 1];
    ByteCount sizes[maxAttributeCount + 1];
    ATSUAttributeValuePtr values[maxAttributeCount + 1];
    int attributeCount = 0;

    Fixed size = FixRatio(emSquare, 1);
    tags[attributeCount] = kATSUSizeTag;
    sizes[attributeCount] = sizeof(size);
    values[attributeCount] = &size;
    ++attributeCount;

    Q_ASSERT(attributeCount < maxAttributeCount + 1);
    OSStatus err = ATSUSetAttributes(unscaledStyle, attributeCount, tags, sizes, values);
    Q_ASSERT(err == noErr);
    Q_UNUSED(err);

    // various CTFont metrics functions: CTFontGetBoundingRectsForGlyphs, CTFontGetAdvancesForGlyphs
    GlyphID atsuGlyph = glyph;
    ATSGlyphScreenMetrics atsuMetrics;
    ATSUGlyphGetScreenMetrics(unscaledStyle, 1, &atsuGlyph, 0,
                              /* iForcingAntiAlias =*/ false,
                              /* iAntiAliasSwitch =*/true,
                              &atsuMetrics);

    metrics->width = int(atsuMetrics.width);
    metrics->height = int(atsuMetrics.height);
    metrics->x = QFixed::fromReal(atsuMetrics.topLeft.x);
    metrics->y = -QFixed::fromReal(atsuMetrics.topLeft.y);
    metrics->xoff = QFixed::fromReal(atsuMetrics.deviceAdvance.x);
    metrics->yoff = QFixed::fromReal(atsuMetrics.deviceAdvance.y);

    QFixedPoint p;
    addGlyphsToPathHelper(unscaledStyle, &glyph, &p, 1, path);

    ATSUDisposeStyle(unscaledStyle);
}
#endif // !QT_MAC_USE_COCOA

QT_END_NAMESPACE
