/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "BumpTop/ArrayOnStack.h"
#include "BumpTop/DebugAssert.h"
#include "BumpTop/OSX/CoreTextHelper.h"
#include "BumpTop/QStringHelpers.h"

// http://developer.apple.com/mac/library/documentation/Carbon/Conceptual/CoreText_Programming/Operations/Operations.html  // NOLINT
CTFontRef CreateBoldFont(CTFontRef iFont, Boolean iMakeBold) {
  CTFontSymbolicTraits desiredTrait = 0;
  CTFontSymbolicTraits traitMask;

  // If we are trying to make the font bold, set the desired trait
  // to be bold.
  if (iMakeBold)
    desiredTrait = kCTFontBoldTrait;

  // Mask off the bold trait to indicate that it is the only trait
  // desired to be modified. As CTFontSymbolicTraits is a bit field,
  // we could choose to change multiple traits if we desired.
  traitMask = kCTFontBoldTrait;

  // Create a copy of the original font with the masked trait set to the
  // desired value. If the font family does not have the appropriate style,
  // this will return NULL.

  return CTFontCreateCopyWithSymbolicTraits(iFont, 0.0, NULL, desiredTrait, traitMask);
}

// This is meant to behave similarly to QPainter::drawText, except the offset is slightly different (you won't
//    see items cut off on the left side). Also, it has a flag that lets you choose between using CoreText directly
//    or QPainter; we found that QPainter's kerning was weird on the Mac
void drawNativeText(QPainter* painter, int x, int y, QString text, QColor background_color) {
  if (text == "") {
    return;
  }
  QFontMetrics metrics(painter->font());
  QRect text_rect = metrics.boundingRect(text);
  if (text_rect.width() == 0 || text_rect.height() == 0) {
    return;
  }

  bool use_native_text_rendering = true;
  if (use_native_text_rendering) {
    size_t cg_buffer_size = text_rect.width()*text_rect.height()*4;
    unsigned char *cg_buffer = ARRAY_ON_STACK(unsigned char, cg_buffer_size);
    for (int i = 0; i < text_rect.width()*text_rect.height()*4; i += 4) {
      cg_buffer[i + 0] = 0;
      cg_buffer[i + 1] = background_color.red();
      cg_buffer[i + 2] = background_color.green();
      cg_buffer[i + 3] = background_color.blue();
    }
    CGColorSpaceRef rgbColorSpace = [[NSColorSpace genericRGBColorSpace] CGColorSpace];
    assert(rgbColorSpace != NULL);

    CGContextRef context = CGBitmapContextCreate(cg_buffer,  // data - pass NULL to let CG allocate the memory
                                                 text_rect.width(),
                                                 text_rect.height(),
                                                 8,  // bitsPerComponent
                                                 4*text_rect.width(),  // bytesPerRow
                                                 rgbColorSpace,
                                                 kCGImageAlphaPremultipliedFirst);
    assert(context != NULL);

    // TODO: Apparently, to enable subpixel rendering, the last parameter should be
    // "kCGBitmapByteOrder32Host|kCGImageAlphaPremultipliedFirst". However, when I do this, the text comes out looking
    // really blocky and stuff. Perhaps I'm not converting the bitmap correctly still? I should investigate this...
    // perhaps render on white and zoom in to see if pixels are colored or not

    // Prepare font
    CTFontDescriptorRef ct_font_descriptor = CTFontDescriptorCreateWithNameAndSize(CFStringFromQString(painter->font().family()),  // NOLINT
                                                                                   painter->font().pointSize());
    assert(ct_font_descriptor != NULL);

    CTFontRef ct_font = CTFontCreateWithFontDescriptor(ct_font_descriptor, painter->font().pointSize(), NULL);

    // trying another method because the previous method was sometimes returning a NULL value, oddly enough
    // we will remove this if it doesn't do anything
    // if (ct_font == NULL) {
    //   ct_font = CTFontCreateUIFontForLanguage(kCTFontSystemFontType, painter->font().pointSize(), NULL);
    // }

    assert(ct_font != NULL);

    if (painter->font().bold()) {
      CTFontRef ct_bold_font = CreateBoldFont(ct_font, true);
      DEBUG_ASSERT(ct_bold_font != NULL);
      CFRelease(ct_font);
      if (ct_bold_font != NULL) {
        ct_font = ct_bold_font;
      }
      assert(ct_font != NULL);
    }

    // Create an attributed string
    const QColor& pen_color = painter->pen().color();
    CGFloat components[] = { pen_color.redF(), pen_color.greenF(), pen_color.blueF(), pen_color.alphaF() };
    CGColorRef cg_pen_color = CGColorCreate(rgbColorSpace, components);
    assert(cg_pen_color != NULL);
    CGColorSpaceRelease(rgbColorSpace);

    CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
    CFTypeRef values[] = { ct_font, cg_pen_color };

    CFDictionaryRef attr = CFDictionaryCreate(kCFAllocatorDefault, (const void **)&keys, (const void **)&values,
                                              sizeof(keys) / sizeof(keys[0]),
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, CFStringFromQString(text), attr);
    CFRelease(attr);

    // Draw the string
    CTLineRef line = CTLineCreateWithAttributedString(attrString);
    CFRelease(attrString);
    CGContextSetTextMatrix(context, CGAffineTransformIdentity);
    // metrics.descent() + 2 is the magic factor that aligns this with our Qt-driven text rendering
    //   (its a kludge that works for font sizes < 50)
    CGContextSetTextPosition(context, 0, metrics.descent() + 2 - 1);
    CTLineDraw(line, context);

    QImage text_image = QImage(text_rect.width(), text_rect.height(), QImage::Format_ARGB32);

    unsigned char* source_data = cg_buffer;
    unsigned char* dest_data = text_image.bits();
    for (int i = 0; i < cg_buffer_size; i += 4) {
#if TARGET_RT_BIG_ENDIAN
      // PPC
      dest_data[i + 0] = source_data[i + 0];
      dest_data[i + 1] = source_data[i + 1];
      dest_data[i + 2] = source_data[i + 2];
      dest_data[i + 3] = source_data[i + 3];
#else
      // Intel
      dest_data[i + 0] = source_data[i + 3];
      dest_data[i + 1] = source_data[i + 2];
      dest_data[i + 2] = source_data[i + 1];
      dest_data[i + 3] = source_data[i + 0];
#endif
    }

    painter->drawImage(x, y, text_image);

    CFRelease(line);
    CGContextRelease(context);
  } else {
    painter->drawText(x + 1, y - text_rect.y() + 1, text);
  }
}
