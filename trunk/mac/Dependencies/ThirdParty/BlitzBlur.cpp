
/*
 Copyright (C) 1998, 1999, 2001, 2002, 2004, 2005, 2007
 Daniel M. Duley <daniel.duley@verizon.net>
 (C) 2004 Zack Rusin <zack@kde.org>
 (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
 (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
 (C) 1998, 1999 Dirk Mueller <mueller@kde.org>

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/*
 Portions of this software are were originally based on ImageMagick's
 algorithms. ImageMagick is copyrighted under the following conditions:

 Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
 making software imaging solutions freely available.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files ("ImageMagick"), to deal
 in ImageMagick without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
 copies of ImageMagick, and to permit persons to whom the ImageMagick is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of ImageMagick.

 The software is provided "as is", without warranty of any kind, express or
 implied, including but not limited to the warranties of merchantability,
 fitness for a particular purpose and noninfringement.  In no event shall
 ImageMagick Studio be liable for any claim, damages or other liability,
 whether in an action of contract, tort or otherwise, arising from, out of or
 in connection with ImageMagick or the use or other dealings in ImageMagick.

 Except as contained in this notice, the name of the ImageMagick Studio shall
 not be used in advertising or otherwise to promote the sale, use or other
 dealings in ImageMagick without prior written authorization from the
 ImageMagick Studio.
 */

/*
 *  Note: this file was modified as part of BumpTop
 *
 */

#include "ThirdParty/BlitzBlur.h"

#include <string.h>

#include "BumpTop/Math.h"

QRgb BlitzPrivate::convertFromPremult(QRgb p)
{
  int alpha = qAlpha(p);
  return(!alpha ? 0 : qRgba(255*qRed(p)/alpha,
                            255*qGreen(p)/alpha,
                            255*qBlue(p)/alpha,
                            alpha));
}

QImage Blitz::blur(QImage &img, int radius)
{
  QRgb *p1, *p2;
  int x, y, w, h, mx, my, mw, mh, mt, xx, yy;
  int a, r, g, b;
  int *as, *rs, *gs, *bs;

  if(radius < 1 || img.isNull() || img.width() < (radius << 1))
    return(img);

  w = img.width();
  h = img.height();

  if(img.depth() < 8)
    img = img.convertToFormat(QImage::Format_Indexed8);
  QImage buffer(w, h, img.hasAlphaChannel() ?
                QImage::Format_ARGB32 : QImage::Format_RGB32);

  as = new int[w];
  rs = new int[w];
  gs = new int[w];
  bs = new int[w];

  QVector<QRgb> colorTable;
  if(img.format() == QImage::Format_Indexed8)
    colorTable = img.colorTable();

  for(y = 0; y < h; y++){
    my = y - radius;
    mh = (radius << 1) + 1;
    if(my < 0){
      mh += my;
      my = 0;
    }
    if((my + mh) > h)
      mh = h - my;

    p1 = (QRgb *)buffer.scanLine(y);
    memset(as, 0, w * sizeof(int));
    memset(rs, 0, w * sizeof(int));
    memset(gs, 0, w * sizeof(int));
    memset(bs, 0, w * sizeof(int));

    if(img.format() == QImage::Format_ARGB32_Premultiplied){
      QRgb pixel;
      for(yy = 0; yy < mh; yy++){
        p2 = (QRgb *)img.scanLine(yy + my);
        for(x = 0; x < w; x++, p2++){
          pixel = BlitzPrivate::convertFromPremult(*p2);
          as[x] += qAlpha(pixel);
          rs[x] += qRed(pixel);
          gs[x] += qGreen(pixel);
          bs[x] += qBlue(pixel);
        }
      }
    }
    else if(img.format() == QImage::Format_Indexed8){
      QRgb pixel;
      unsigned char *ptr;
      for(yy = 0; yy < mh; yy++){
        ptr = (unsigned char *)img.scanLine(yy + my);
        for(x = 0; x < w; x++, ptr++){
          pixel = colorTable[*ptr];
          as[x] += qAlpha(pixel);
          rs[x] += qRed(pixel);
          gs[x] += qGreen(pixel);
          bs[x] += qBlue(pixel);
        }
      }
    }
    else{
      for(yy = 0; yy < mh; yy++){
        p2 = (QRgb *)img.scanLine(yy + my);
        for(x = 0; x < w; x++, p2++){
          as[x] += qAlpha(*p2);
          rs[x] += qRed(*p2);
          gs[x] += qGreen(*p2);
          bs[x] += qBlue(*p2);
        }
      }
    }

    for(x = 0; x < w; x++){
      a = r = g = b = 0;
      mx = x - radius;
      mw = (radius << 1) + 1;
      if(mx < 0){
        mw += mx;
        mx = 0;
      }
      if((mx + mw) > w)
        mw = w - mx;
      mt = mw * mh;
      for(xx = mx; xx < (mw + mx); xx++) {
        a += as[xx];
        r += rs[xx];
        g += gs[xx];
        b += bs[xx];
      }
      a = a / mt;
      r = r / mt;
      g = g / mt;
      b = b / mt;
      *p1++ = qRgba(r, g, b, a);
    }
  }
  delete[] as;
  delete[] rs;
  delete[] gs;
  delete[] bs;

  return(buffer);
}

QImage Blitz::convertShadowOverlayForUseWithOgre(QImage &img) {
  QRgb p1, p2;

  int w = img.width();
  int h = img.height();

  if(img.depth() < 8)
    img = img.convertToFormat(QImage::Format_Indexed8);

  QImage buffer(w, h, img.hasAlphaChannel() ?
                QImage::Format_ARGB32 : QImage::Format_RGB32);

  QVector<QRgb> colorTable;
  if(img.format() == QImage::Format_Indexed8)
    colorTable = img.colorTable();

  for(int y = 0; y < h; y++) {
    for(int x = 0; x < w; x++){
      p1 = img.pixel(x,y);
      double red = std::min(255-qAlpha(p1) + ((double) qRed(p1))*(1.0*qAlpha(p1)/255.0), 255.0);
      double green = std::min(255-qAlpha(p1) + ((double) qGreen(p1))*(1.0*qAlpha(p1)/255.0), 255.0);
      double blue = std::min(255-qAlpha(p1) + ((double) qBlue(p1))*(1.0*qAlpha(p1)/255.0), 255.0);

      p2 = qRgba( round(red), round(green), round(blue), 255);
      buffer.setPixel(x,y,p2);
    }
  }
  return(buffer);
}

