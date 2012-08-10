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

#include <gtest/gtest.h>

#include "BumpTop/ArrayOnStack.h"
#include "BumpTop/RC4.h"

namespace {

  // The fixture for testing class Foo.
  class RC4Test : public ::testing::Test {
  };

  // Tests that the Foo::Bar() method does Abc.
  TEST_F(RC4Test, RC4HashIsCorrect) {
    char* key1 = "Key";
    char* message1 = "Plaintext";
    char* ciphered_message1 = ARRAY_ON_STACK(char, strlen(message1) + 1);
    strcpy(ciphered_message1, message1);  // NOLINT
    RC4Encipher(key1, strlen(key1), ciphered_message1, strlen(ciphered_message1));
    EXPECT_STREQ("\xBB\xF3\x16\xE8\xD9\x40\xAF\x0A\xD3", ciphered_message1);

    char* key2 = "Wiki";
    char* message2 = "pedia";
    char* ciphered_message2 = ARRAY_ON_STACK(char, strlen(message2) + 1);
    strcpy(ciphered_message2, message2);  // NOLINT
    RC4Encipher(key2, strlen(key2), ciphered_message2, strlen(ciphered_message2));
    EXPECT_STREQ("\x10\x21\xBF\x04\x20", ciphered_message2);

    char* key3 = "Secret";
    char* message3 = "Attack at dawn";
    char* ciphered_message3 = ARRAY_ON_STACK(char, strlen(message3) + 1);
    strcpy(ciphered_message3, message3);  // NOLINT
    RC4Encipher(key3, strlen(key3), ciphered_message3, strlen(ciphered_message3));
    EXPECT_STREQ("\x45\xA0\x1F\x64\x5F\xC3\x5B\x38\x35\x52\x54\x4B\x9B\xF5", ciphered_message3);
  }

}  // namespace
