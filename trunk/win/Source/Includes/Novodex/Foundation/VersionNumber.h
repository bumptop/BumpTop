/*
VersionNumbers:  The combination of these
numbers uniquely identifies the API, and should
be incremented when the SDK API changes.  This may
include changes to file formats.

This header is included in the main SDK header files
so that the entire SDK and everything that builds on it 
is completely rebuilt when this file changes.  Thus, 
this file is not to include a frequently changing 
build number.  See BuildNumber.h for that.

Each of these three values should stay below 255 because
sometimes they are stored in a byte.
*/

#define NX_SDK_VERSION_MAJOR 2
#define NX_SDK_VERSION_MINOR 1
#define NX_SDK_VERSION_BUGFIX 1
