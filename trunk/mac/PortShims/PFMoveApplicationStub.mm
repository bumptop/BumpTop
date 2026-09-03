// No-op replacement for LetsMove's PFMoveToApplicationsFolderIfNecessary().
// The original prompts to relocate the app into /Applications using
// Security-framework APIs removed from modern macOS; the port just runs from
// wherever it is.

#import "ThirdParty/PFMoveApplication/PFMoveApplication.h"

void PFMoveToApplicationsFolderIfNecessary(void) {
}
