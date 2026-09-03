// No-op implementation of the Sparkle SUUpdater stub (see Sparkle/Sparkle.h).

#import "Sparkle/Sparkle.h"

@implementation SUUpdater

+ (SUUpdater *)sharedUpdater {
  static SUUpdater *shared = nil;
  if (shared == nil)
    shared = [[SUUpdater alloc] init];
  return shared;
}

- (BOOL)sendsSystemProfile { return NO; }
- (void)setSendsSystemProfile:(BOOL)value { (void)value; }
- (BOOL)automaticallyDownloadsUpdates { return NO; }
- (void)setAutomaticallyDownloadsUpdates:(BOOL)value { (void)value; }
- (BOOL)automaticallyChecksForUpdates { return NO; }
- (void)setAutomaticallyChecksForUpdates:(BOOL)value { (void)value; }
- (IBAction)checkForUpdates:(id)sender { (void)sender; }

@end
