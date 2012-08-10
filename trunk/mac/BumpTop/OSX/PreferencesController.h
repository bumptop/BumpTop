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

#import <Cocoa/Cocoa.h>


@interface PreferencesController : NSWindowController {
  IBOutlet NSView* preferences_settings_view_;
  NSToolbar* preferences_toolbar_;

  NSString* current_identifier_;
  bool is_first_time_loading_;

  NSMutableArray* identifiers_;
  NSMutableDictionary* identifiers_and_views_;
  NSMutableDictionary* identifiers_and_items_;

  NSViewAnimation* view_animation_;
}

-(void)showWindowWithView:(NSString*) identifier;
-(void)startChangeView:(NSToolbarItem *)toolbarItem;
-(void)changeView:(NSString *)identifier withIsAnimationEnabled:(bool)is_animation_enabled;
-(NSRect)windowFrameForView:(NSView *)view;
-(void)showWindowWithView:(NSString *)identifier;
-(void)beginSlideInAndOutAnimation:(NSView *)new_view;
-(void)initializeIdentifiersAndViews;
-(void)initializeToolbarItems;
-(void)initializeToolbar;
+(PreferencesController*)singleton;

@end
