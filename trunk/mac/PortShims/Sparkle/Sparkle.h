// Stub replacing the Sparkle auto-update framework for the modern macOS port.
// The original update feed (macstats.bumptop.com) is long dead; this keeps the
// call sites compiling while making every operation a no-op.

#ifndef PORTSHIMS_SPARKLE_SPARKLE_H_
#define PORTSHIMS_SPARKLE_SPARKLE_H_

#import <Foundation/Foundation.h>

@interface SUUpdater : NSObject
+ (SUUpdater *)sharedUpdater;
- (BOOL)sendsSystemProfile;
- (void)setSendsSystemProfile:(BOOL)value;
- (BOOL)automaticallyDownloadsUpdates;
- (void)setAutomaticallyDownloadsUpdates:(BOOL)value;
- (BOOL)automaticallyChecksForUpdates;
- (void)setAutomaticallyChecksForUpdates:(BOOL)value;
- (IBAction)checkForUpdates:(id)sender;
@end

#endif  // PORTSHIMS_SPARKLE_SPARKLE_H_
