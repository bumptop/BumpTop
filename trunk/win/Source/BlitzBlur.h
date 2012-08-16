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

namespace Blitz {
  QImage blur(QImage &img, int radius);
}

namespace BlitzPrivate {
  QRgb convertFromPremult(QRgb p);
}

