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

#import "BumpTop/OSX/PreferencesController.h"

#import "BumpTop/OSX/GeneralSettingsController.h"
#import "BumpTop/OSX/ProSettingsController.h"
#import "BumpTop/OSX/TextureSettingsController.h"

#include "BumpTop/AppSettings.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/QStringHelpers.h"

static PreferencesController *singleton_ = NULL;

@implementation PreferencesController

-(id)init {
  if (![super initWithWindowNibName:@"PreferencesWindow"])
    return nil;

  view_animation_ = [[NSViewAnimation alloc] init];
  [view_animation_ setAnimationBlockingMode:NSAnimationNonblocking];
  [view_animation_ setAnimationCurve:NSAnimationEaseInOut];
  [view_animation_ setDelegate:(id<NSAnimationDelegate>)self];
  AppSettings::singleton()->loadSettingsFile();
  assert(singleton_ == NULL);
  singleton_ = self;
  is_first_time_loading_ = true;
  return self;
}

-(void)windowDidLoad {
}

-(void)startChangeView:(NSToolbarItem *)toolbarItem {
  [self changeView:[toolbarItem itemIdentifier] withIsAnimationEnabled:true];
}

// http://www.mere-mortal-software.com/blog/details.php?d=2007-03-14
-(void)changeView:(NSString *)identifier withIsAnimationEnabled:(bool)is_animation_enabled{
  if ([identifier isEqualToString:current_identifier_]) {
    return;
  }

  [preferences_toolbar_ setSelectedItemIdentifier:identifier];
  current_identifier_ = identifier;
  AppSettings::singleton()->set_preferences_window_default_view(utf8(QStringFromNSString(current_identifier_)));
  AppSettings::singleton()->saveSettingsFile();

  NSView* new_view = [identifiers_and_views_ objectForKey:identifier];

  // remove subviews from preference_settings_view_ until only one subview is left
  NSView *current_view = nil;
  if ([[preferences_settings_view_ subviews] count] > 0) {
    NSEnumerator *subviewsEnum = [[preferences_settings_view_ subviews] reverseObjectEnumerator];
    current_view = [subviewsEnum nextObject];
    NSView* subview = [subviewsEnum nextObject];
    while (subview != nil) {
      [subview removeFromSuperviewWithoutNeedingDisplay];
      subview = [subviewsEnum nextObject];
    }
  }

  NSRect frame = [new_view bounds];
  frame.origin.y = NSHeight([preferences_settings_view_ frame]) - NSHeight([new_view bounds]);
  [new_view setFrame:frame];
  [current_view removeFromSuperviewWithoutNeedingDisplay];
  if (is_animation_enabled) {
    [self beginSlideInAndOutAnimation:new_view];
  } else {
    [preferences_settings_view_ addSubview:new_view];
    [new_view setHidden:NO];
    [[self window] setFrame:[self windowFrameForView:new_view] display:YES];
  }

  [[self window] setTitle:current_identifier_];
}

-(NSRect)windowFrameForView:(NSView *)view {
  NSRect window_frame = [[self window] frame];
  NSRect content_rect = [[self window] contentRectForFrameRect:window_frame];
  float window_title_and_toolbar_height = NSHeight(window_frame) - NSHeight(content_rect);

  window_frame.size.height = NSHeight([view frame]) + window_title_and_toolbar_height;
  window_frame.size.width = NSWidth([view frame]);
  window_frame.origin.y = NSMaxY([[self window] frame]) - NSHeight(window_frame);

  return window_frame;
}

-(void)beginSlideInAndOutAnimation:(NSView *)new_view {
  [view_animation_ stopAnimation];

  [view_animation_ setDuration:0.25];

  NSDictionary *resize_dictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [self window], NSViewAnimationTargetKey,
                                    [NSValue valueWithRect:[[self window] frame]], NSViewAnimationStartFrameKey,
                                    [NSValue valueWithRect:[self windowFrameForView:new_view]], NSViewAnimationEndFrameKey,
                                    nil];

  NSArray *animation_array = [NSArray arrayWithObjects:
                             resize_dictionary,
                             nil];

  [view_animation_ setViewAnimations:animation_array];
  [view_animation_ startAnimation];
}

-(void)animationDidEnd:(NSAnimation *)animation {
  [preferences_settings_view_ addSubview:[identifiers_and_views_ objectForKey:current_identifier_]];
  [[identifiers_and_views_ objectForKey:current_identifier_] setHidden:NO];
  [[self window] setFrame:[self windowFrameForView:[identifiers_and_views_ objectForKey:current_identifier_]] display:YES];
}

-(IBAction)showWindow:(id)sender {
  (void)[self window];
  [[self window] orderOut:nil];

  if(is_first_time_loading_) {
    [self initializeIdentifiersAndViews];
    [self initializeToolbarItems];
    [self initializeToolbar];
    is_first_time_loading_ = false;
  }
  if (AppSettings::singleton()->preferences_window_default_view() == "Pro")
    AppSettings::singleton()->set_preferences_window_default_view("General");
  if ([NSStringFromUtf8(AppSettings::singleton()->preferences_window_default_view()) isEqualToString:@"Backgrounds"])
    [preferences_toolbar_ setSelectedItemIdentifier:@"Backgrounds"];
  else if ([NSStringFromUtf8(AppSettings::singleton()->preferences_window_default_view()) isEqualToString:@"Backgrounds"])
    [preferences_toolbar_ setSelectedItemIdentifier:@"General"];
  else
    [preferences_toolbar_ setSelectedItemIdentifier:@"Pro"];
  [self changeView:NSStringFromUtf8(AppSettings::singleton()->preferences_window_default_view()) withIsAnimationEnabled:false];

  [super showWindow:sender];
  [[self window] makeKeyAndOrderFront:nil];
  [[GeneralSettingsController singleton] prepareForDisplay];
  //[[ProSettingsController singleton] prepareForDisplay];
  BumpTopApp::makeSelfForegroundApp();
}

-(void)showWindowWithView:(NSString *)identifier {
  AppSettings::singleton()->set_preferences_window_default_view("Backgrounds");
  AppSettings::singleton()->saveSettingsFile();
  [self showWindow:nil];
}

-(void)initializeIdentifiersAndViews {
  identifiers_and_views_ = [[NSMutableDictionary alloc] init];
  [identifiers_and_views_ setObject:[[GeneralSettingsController singleton] view] forKey:@"General"];
  [identifiers_and_views_ setObject:[[TextureSettingsController singleton] view] forKey:@"Backgrounds"];
  [[TextureSettingsController singleton] showInWindow:[self window]];
  //[identifiers_and_views_ setObject:[[ProSettingsController singleton] view] forKey:@"Pro"];
}

-(void)initializeToolbar {
  preferences_toolbar_ = [[NSToolbar alloc] initWithIdentifier:@"PreferencesToolbar"];
  [preferences_toolbar_ setAllowsUserCustomization:NO];
  [preferences_toolbar_ setAutosavesConfiguration:NO];
  [preferences_toolbar_ setSizeMode:NSToolbarSizeModeDefault];
  [preferences_toolbar_ setDisplayMode:NSToolbarDisplayModeIconAndLabel];
  [preferences_toolbar_ setDelegate:(id<NSToolbarDelegate>)self];
  [[self window] setToolbar:preferences_toolbar_];
}

-(void)initializeToolbarItems {
  identifiers_ = [[NSMutableArray alloc] init];
  identifiers_and_items_ = [[NSMutableDictionary alloc] init];

  NSImage *general_item_image = [[NSImage alloc] initWithContentsOfFile:NSStringFromQString(FileManager::pathForResource("prefs_general.png"))];
  NSToolbarItem *general_item = [[[NSToolbarItem alloc] initWithItemIdentifier:@"General"] autorelease];
  [general_item setLabel:@"General"];
  [general_item setImage:general_item_image];
  [general_item setTarget:self];
  [general_item setAction:@selector(startChangeView:)];

  [identifiers_ addObject:@"General"];
  [identifiers_and_items_ setObject:general_item forKey:@"General"];

  NSImage *background_item_image = [[NSImage alloc] initWithContentsOfFile:NSStringFromQString(FileManager::pathForResource("prefs_backgrounds.png"))];
  NSToolbarItem *background_item = [[[NSToolbarItem alloc] initWithItemIdentifier:@"Backgrounds"] autorelease];
  [background_item setLabel:@"Backgrounds"];
  [background_item setImage:background_item_image];
  [background_item setTarget:self];
  [background_item setAction:@selector(startChangeView:)];

  [identifiers_ addObject:@"Backgrounds"];
  [identifiers_and_items_ setObject:background_item forKey:@"Backgrounds"];


  /*NSImage *pro_item_image = [[NSImage alloc] initWithContentsOfFile:NSStringFromQString(FileManager::pathForResource("prefs_pro.png"))];
  NSToolbarItem *pro_item = [[[NSToolbarItem alloc] initWithItemIdentifier:@"Pro"] autorelease];
  [pro_item setLabel:@"Pro"];
  [pro_item setImage:pro_item_image];
  [pro_item setTarget:self];
  [pro_item setAction:@selector(startChangeView:)];

  [identifiers_ addObject:@"Pro"];
  [identifiers_and_items_ setObject:pro_item forKey:@"Pro"];*/

}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar*)toolbar {
  return identifiers_;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar*)toolbar  {
  return identifiers_;
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar {
  return identifiers_;
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSString *)identifier willBeInsertedIntoToolbar:(BOOL)willBeInserted {
  return [identifiers_and_items_ objectForKey:identifier];
}

+(PreferencesController*)singleton {
  return singleton_;
}

@end
