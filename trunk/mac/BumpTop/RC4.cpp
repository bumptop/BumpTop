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

#include <algorithm>

#include "BumpTop/ArrayOnStack.h"
#include "BumpTop/RC4.h"

void swap(unsigned char* a, unsigned char* b) {
  unsigned char temp = *a;
  *a = *b;
  *b = temp;
}

void RC4KeySchedulingAlgorithm(const unsigned char* key, size_t key_len, unsigned char* S) {
  for (int i = 0; i < 256; i++)
    S[i] = i;
  unsigned char j = 0;
  for (int i = 0; i < 256; i++) {
    j = (j + S[i] + key[i % key_len]) % 256;
    swap(&S[i], &S[j]);
  }
}

void RC4Encipher(const char* key, size_t key_len, char* message, size_t message_len) {
  unsigned char S[256];
  RC4KeySchedulingAlgorithm((unsigned char*)key, key_len, S);

  unsigned char i = 0;
  unsigned char j = 0;
  for (int c = 0; c < message_len; c++) {
    i = (i + 1) % 256;
    j = (j + S[i]) % 256;
    swap(&S[i], &S[j]);
    message[c] = message[c] ^ S[(S[i] + S[j]) % 256];
  }
}

QByteArray RC4Encipher(QByteArray key, QByteArray data) {
  RC4Encipher(key.constData(), key.size(), data.data(), data.size());
  return data;
}

QByteArray RC4Decipher(QByteArray key, QByteArray data) {
  return RC4Encipher(key, data);
}
