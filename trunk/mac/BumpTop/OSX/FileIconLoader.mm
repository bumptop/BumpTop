//
//  Copyright 2012 Google Inc. All Rights Reserved.
//  
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#include "BumpTop/OSX/FileIconLoader.h"

#import <QuickLook/QuickLook.h>
#import <AppKit/NSWorkspace.h>

#include "BumpTop/Authorization.h"
#include "BumpTop/FileItem.h"
#include "BumpTop/MaterialLoader.h"
#include "BumpTop/QStringHelpers.h"

FileIconLoader::FileIconLoader(const QString& file_path, MaterialLoader::IconLoadMethod icon_load_method)
: file_path_(file_path),
  icon_load_method_(icon_load_method) {
  icon_.width = 0;
  icon_.height = 0;
  icon_.image_data = NULL;
}

FileIconLoader::~FileIconLoader() {
  if (icon_.image_data != NULL) {
    delete[] icon_.image_data;
  }
}

void FileIconLoader::loadResource(Ogre::Resource *resource) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  icon_ = iconBitmapForPath(file_path_, 200, icon_load_method_);
  [pool release];

  if (icon_.image_data != NULL) {
    Ogre::Image image = Ogre::Image();
#if TARGET_RT_BIG_ENDIAN
    Ogre::PixelFormat pixel_format = Ogre::PF_R8G8B8A8;
#else
    Ogre::PixelFormat pixel_format = Ogre::PF_A8B8G8R8;
#endif  // TARGET_RT_BIG_ENDIAN
    image.loadDynamicImage(icon_.image_data, icon_.width, icon_.height, pixel_format);
    Ogre::ConstImagePtrList imagePtrs;
    imagePtrs.push_back(&image);

    static_cast<Ogre::Texture*>(resource)->_loadImages(imagePtrs);
  }
}

#if defined(OS_WIN)
#else
CGImageRef iconForContextMenu(QString path) {
  NSImage *icon_image = [[NSWorkspace sharedWorkspace] iconForFile:NSStringFromQString(path)];

  int desiredPixelWidth = 16;
  int desiredPixelHeight = 16;
  // http://www.cocoadev.com/index.pl?CGImageRef
  // The default size for the icons is 32x32; here we get a scaling factor required to make the icons the desired size
  CGContextRef bitmapCtx = CGBitmapContextCreate(NULL/*data - pass NULL to let CG allocate the memory*/,
                                                 desiredPixelWidth,
                                                 desiredPixelHeight,
                                                 8 /*bitsPerComponent*/,
                                                 0 /*bytesPerRow - CG will calculate it for you if it's allocating the data.  This might get padded out a bit for better alignment*/,
                                                 [[NSColorSpace genericRGBColorSpace] CGColorSpace],
                                                 kCGBitmapByteOrder32Host|kCGImageAlphaPremultipliedFirst);

  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:bitmapCtx flipped:NO]];
  [icon_image drawInRect:NSMakeRect(0,0, desiredPixelWidth, desiredPixelHeight)
                fromRect:NSZeroRect/*sentinel, means "the whole thing*/
               operation:NSCompositeCopy fraction:1.0];
  [NSGraphicsContext restoreGraphicsState];

  CGImageRef icon_image_ref = CGBitmapContextCreateImage(bitmapCtx);
  CGContextRelease(bitmapCtx);
  return icon_image_ref;
}

// This function returns a BitmapImage corresponding to the icon for a given path
BitmapImage iconBitmapForPath(QString path, int icon_size, MaterialLoader::IconLoadMethod icon_load_method) {
  // check whether Quick Look is available on our system
  // you need to save it to an intermediate variable, and you need to cast it to uint32-- or else it fails
  // why? i have no fucking clue
  uint32 quick_look_fn_pointer = (uint32)QLThumbnailImageCreate;
  bool quick_look_supported = (quick_look_fn_pointer != 0);

  CGImageRef quick_look_image_ref;
  BitmapImage icon_bitmap_image;
  NSBitmapImageRep *icon_bitmap = NULL;

  bool attempt_quick_lock = quick_look_supported && (icon_load_method == MaterialLoader::kQuickLook ||
                                                     icon_load_method == MaterialLoader::kQuickLookWithStandardIconFallback);
  if (attempt_quick_lock) {
    NSString *ns_path = NSStringFromQString(path);
    NSURL *url_path = [NSURL fileURLWithPath:ns_path];

    // Try to get a QuickLook Icon
    NSDictionary* quick_arguments_dict = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES]
                                                                     forKey:(NSString *)kQLThumbnailOptionIconModeKey];

    quick_look_image_ref = QLThumbnailImageCreate(kCFAllocatorDefault,
                                                  (CFURLRef)url_path,
                                                  CGSizeMake(icon_size, icon_size),
                                                  (CFDictionaryRef)quick_arguments_dict);
  }

  // If QuickLook fails, get the standard icon
  if (attempt_quick_lock && quick_look_image_ref) {
    icon_bitmap = [[NSBitmapImageRep alloc] initWithCGImage: quick_look_image_ref];
  } else if (icon_load_method == MaterialLoader::kStandardIcon ||
             icon_load_method == MaterialLoader::kQuickLookWithStandardIconFallback) {
    NSImage *icon_image = [[NSWorkspace sharedWorkspace]
                           iconForFile:NSStringFromQString(path)];

    // The default size for the icons is 32x32; here we get a scaling factor required to make the icons the desired size
    double scale_factor = icon_size/(icon_image.size.width*1.0);
    [icon_image setSize:NSMakeSize(round(icon_image.size.width*scale_factor),
                                   round(icon_image.size.height*scale_factor))];

    // Before we were creating a NSBitmapImageRep from the NSImage locked rect; however, this was causing crashes
    // on leopard. Instead, copying into an NSBitmapImageRep as per this URL:
    // http://www.cocoabuilder.com/archive/cocoa/193131-is-lockfocus-main-thread-specific.html#193191
    icon_bitmap = [[NSBitmapImageRep alloc]
                    initWithBitmapDataPlanes:NULL
                    pixelsWide:icon_image.size.width
                    pixelsHigh:icon_image.size.height
                    bitsPerSample:8
                    samplesPerPixel:4
                    hasAlpha:YES
                    isPlanar:NO
                    colorSpaceName:NSCalibratedRGBColorSpace
                    bitmapFormat:0
                    bytesPerRow:0
                    bitsPerPixel:0];

    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:icon_bitmap];
    if (bitmapContext != nil) {
      [NSGraphicsContext saveGraphicsState];
      [NSGraphicsContext setCurrentContext:bitmapContext];

      NSImageInterpolation previousImageInterpolation = [bitmapContext imageInterpolation];
      [bitmapContext setImageInterpolation:NSImageInterpolationHigh];

      [icon_image drawAtPoint:NSZeroPoint fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

      [bitmapContext setImageInterpolation:previousImageInterpolation];
      [NSGraphicsContext restoreGraphicsState];
    }
  }

  if (icon_bitmap != NULL) {
    icon_bitmap_image.height = [icon_bitmap pixelsHigh];
    icon_bitmap_image.width = [icon_bitmap pixelsWide];

    unsigned char* image_data = [icon_bitmap bitmapData];

    int num_bytes = icon_bitmap_image.height * icon_bitmap_image.width * 4;
    assert(image_data != NULL);

    if (num_bytes > 0) {
      icon_bitmap_image.image_data = new unsigned char[num_bytes];
      int bytes_per_row = [icon_bitmap bytesPerRow];
      int width_in_bytes = 4*icon_bitmap_image.width;

      for (int i = 0; i < icon_bitmap_image.height; i++) {
        memcpy(icon_bitmap_image.image_data + i*width_in_bytes,
               image_data + i*bytes_per_row,
               width_in_bytes);
      }
    }
    else {
      icon_bitmap_image.image_data = NULL;
    }

    [icon_bitmap release];
  } else {
    icon_bitmap_image.height = 0;
    icon_bitmap_image.width = 0;
    icon_bitmap_image.image_data = NULL;
  }

  return icon_bitmap_image;
}
#endif
