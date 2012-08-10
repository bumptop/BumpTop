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

#ifndef BUMPTOP_THIRDPARTY_GETPRIMARYMACADDRESS_H_
#define BUMPTOP_THIRDPARTY_GETPRIMARYMACADDRESS_H_

#include <IOKit/IOKitLib.h>

#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

struct MACAddressType {
  UInt8 array[kIOEthernetAddressSize];
};

std::pair<bool, MACAddressType> GetPrimaryMacAddress();
std::pair<bool, QString> GetPrimaryMacAddressString();

#endif BUMPTOP_THIRDPARTY_GETPRIMARYMACADDRESS_H_
