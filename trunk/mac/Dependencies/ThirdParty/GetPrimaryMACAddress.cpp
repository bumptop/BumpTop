// taken from:
// http://developer.apple.com/mac/library/samplecode/GetPrimaryMACAddress/index.html
/*
    File:      GetPrimaryMACAddress.c

    Description:  This sample application demonstrates how to do retrieve the Ethernet MAC
          address of the built-in Ethernet interface from the I/O Registry on Mac OS X.
          Techniques shown include finding the primary (built-in) Ethernet interface,
          finding the parent Ethernet controller, and retrieving properties from the
          controller's I/O Registry entry.

    Copyright:    © Copyright 2001-2005 Apple Computer, Inc. All rights reserved.

    Disclaimer:    IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
          ("Apple") in consideration of your agreement to the following terms, and your
          use, installation, modification or redistribution of this Apple software
          constitutes acceptance of these terms.  If you do not agree with these terms,
          please do not use, install, modify or redistribute this Apple software.

          In consideration of your agreement to abide by the following terms, and subject
          to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
          copyrights in this original Apple software (the "Apple Software"), to use,
          reproduce, modify and redistribute the Apple Software, with or without
          modifications, in source and/or binary forms; provided that if you redistribute
          the Apple Software in its entirety and without modifications, you must retain
          this notice and the following text and disclaimers in all such redistributions of
          the Apple Software.  Neither the name, trademarks, service marks or logos of
          Apple Computer, Inc. may be used to endorse or promote products derived from the
          Apple Software without specific prior written permission from Apple.  Except as
          expressly stated in this notice, no other rights or licenses, express or implied,
          are granted by Apple herein, including but not limited to any patent rights that
          may be infringed by your derivative works or by other works in which the Apple
          Software may be incorporated.

          The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
          WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
          WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
          PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
          COMBINATION WITH YOUR PRODUCTS.

          IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
          CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
          GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
          ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
          OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
          (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
          ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Change History (most recent first):

            <3>     09/15/05  Updated to produce a universal binary. Use kIOMasterPortDefault
                instead of older IOMasterPort function. Print the MAC address
                to stdout in response to <rdar://problem/4021220>.
            <2>    04/30/02  Fix bug in creating the matching dictionary that caused the
                kIOPrimaryInterface property to be ignored. Clean up comments and add
                additional comments about how IOServiceGetMatchingServices operates.
            <1>     06/07/01  New sample.

*/

/*
 *  Note: this file was modified as part of BumpTop
 *
 */

#include "ThirdParty/GetPrimaryMacAddress.h"

#include <stdio.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices, char* interfaceName);
static kern_return_t GetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize);

// Returns an iterator containing the primary (built-in) Ethernet interface. The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t FindEthernetInterfaces(io_iterator_t *matchingServices, char* interfaceName)
{
    kern_return_t    kernResult;
    CFMutableDictionaryRef  matchingDict;
    //CFMutableDictionaryRef  propertyMatchDict;

    // Ethernet interfaces are instances of class kIOEthernetInterfaceClass.
    // IOServiceMatching is a convenience function to create a dictionary with the key kIOProviderClassKey and
    // the specified value.
  // Note by mike: we're not using this method and then filtering by kIOPropertyMatchKey anymore,
  // because that way we were getting authorization errors for some people for whom, for whatever reason,
  // kIOPropertyMatchKey was not TRUE for their en0 interface
    //matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

    matchingDict = IOBSDNameMatching(kIOMasterPortDefault, 0, interfaceName);

    if (NULL == matchingDict) {
      printf("IOBSDNameMatching returned a NULL dictionary.\n");
    }

    // IOServiceGetMatchingServices retains the returned iterator, so release the iterator when we're done with it.
    // IOServiceGetMatchingServices also consumes a reference on the matching dictionary so we don't need to release
    // the dictionary explicitly.
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);
    if (KERN_SUCCESS != kernResult) {
        printf("IOServiceGetMatchingServices returned 0x%08x\n", kernResult);
    }

    return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
// If no interfaces are found the MAC address is set to an empty string.
// In this sample the iterator should contain just the primary interface.
static kern_return_t GetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize)
{
    io_object_t    intfService;
    io_object_t    controllerService;
    kern_return_t  kernResult = KERN_FAILURE;

    // Make sure the caller provided enough buffer space. Protect against buffer overflow problems.
  if (bufferSize < kIOEthernetAddressSize) {
    return kernResult;
  }

  // Initialize the returned address
    bzero(MACAddress, bufferSize);

    // IOIteratorNext retains the returned object, so release it when we're done with it.
    while (intfService = IOIteratorNext(intfIterator))
    {
        CFTypeRef  MACAddressAsCFData;

        // IONetworkControllers can't be found directly by the IOServiceGetMatchingServices call,
        // since they are hardware nubs and do not participate in driver matching. In other words,
        // registerService() is never called on them. So we've found the IONetworkInterface and will
        // get its parent controller by asking for it specifically.

        // IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
        kernResult = IORegistryEntryGetParentEntry(intfService,
                           kIOServicePlane,
                           &controllerService);

        if (KERN_SUCCESS != kernResult) {
            printf("IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
        }
        else {
            // Retrieve the MAC address property from the I/O Registry in the form of a CFData
            MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService,
                                 CFSTR(kIOMACAddress),
                                 kCFAllocatorDefault,
                                 0);
            if (MACAddressAsCFData) {
                CFShow(MACAddressAsCFData); // for display purposes only; output goes to stderr

                // Get the raw bytes of the MAC address from the CFData
                CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
                CFRelease(MACAddressAsCFData);
            }

            // Done with the parent Ethernet controller object so we release it.
            (void) IOObjectRelease(controllerService);
        }

        // Done with the Ethernet interface object so we release it.
        (void) IOObjectRelease(intfService);
    }

    return kernResult;
}

std::pair<bool, MACAddressType> GetPrimaryMacAddress() {
  kern_return_t  kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
  /*
   *  error number layout as follows (see mach/error.h and IOKit/IOReturn.h):
   *
   *  hi                lo
   *  | system(6) | subsystem(12) | code(14) |
   */

  io_iterator_t  intfIterator;
  MACAddressType mac_address = {0};

  kernResult = FindEthernetInterfaces(&intfIterator, "en0");

  if (KERN_SUCCESS != kernResult) {
    //printf("FindEthernetInterfaces returned 0x%08x\n", kernResult);
    (void) IOObjectRelease(intfIterator);  // Release the iterator.
    return std::pair<bool, MACAddressType>(false, mac_address);
  }
  else {
    kernResult = GetMACAddress(intfIterator, mac_address.array, sizeof(MACAddressType));

    if (KERN_SUCCESS != kernResult) {
      kernResult = FindEthernetInterfaces(&intfIterator, "en1");
      
      if (KERN_SUCCESS != kernResult) {
        //printf("FindEthernetInterfaces returned 0x%08x\n", kernResult);
        (void) IOObjectRelease(intfIterator);  // Release the iterator.
        return std::pair<bool, MACAddressType>(false, mac_address);
      }
      else {
        kernResult = GetMACAddress(intfIterator, mac_address.array, sizeof(MACAddressType));
        
        if (KERN_SUCCESS != kernResult) {
          //printf("GetMACAddress returned 0x%08x\n", kernResult);
          (void) IOObjectRelease(intfIterator);  // Release the iterator.
          return std::pair<bool, MACAddressType>(false, mac_address);
        }
        else {
          (void) IOObjectRelease(intfIterator);  // Release the iterator.
          return std::pair<bool, MACAddressType>(true, mac_address);
        }
      }
      
    }
    else {
      (void) IOObjectRelease(intfIterator);  // Release the iterator.
      return std::pair<bool, MACAddressType>(true, mac_address);
    }
  }
}

std::pair<bool, QString> GetPrimaryMacAddressString() {
  //return std::pair<bool, QString>(true, "001ff3ccaf76");
  std::pair<bool, MACAddressType> bool_and_mac_address = GetPrimaryMacAddress();
  if (!bool_and_mac_address.first) {
    return std::pair<bool, QString>(false, "");
  } else {
    return std::pair<bool, QString>(true,
                                    QString("").sprintf("%02x%02x%02x%02x%02x%02x",
                                                        bool_and_mac_address.second.array[0],
                                                        bool_and_mac_address.second.array[1],
                                                        bool_and_mac_address.second.array[2],
                                                        bool_and_mac_address.second.array[3],
                                                        bool_and_mac_address.second.array[4],
                                                        bool_and_mac_address.second.array[5]));
  }
}
