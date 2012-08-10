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

#include <fstream> // NOLINT
#include <string>

#include "BumpTop/protoc/AllMessages.pb.h"
#include "BumpTop/RC4.h"

int main(int argc, char *argv[]) {
  ProConfig pro_config_file;
  pro_config_file.add_sticky_note_sizes(200);  // first note
  pro_config_file.add_sticky_note_sizes(200);  // second note
  pro_config_file.add_sticky_note_sizes(200);  // third note
  pro_config_file.set_flip_pile_scroll_delta_threshold(0.1);
  pro_config_file.set_flip_pile_scroll_index_advancement(1);
  pro_config_file.set_find_as_you_type_y_displacement(45);
  pro_config_file.set_find_as_you_type_visible_alpha_value(1.0);
  pro_config_file.set_find_as_you_type_r_tint(0.2);
  pro_config_file.set_find_as_you_type_g_tint(0.6);
  pro_config_file.set_find_as_you_type_b_tint(0.2);
  pro_config_file.set_find_as_you_type_selected_value(1);
  pro_config_file.set_pinch_gesture_threshold(0.7);
  pro_config_file.set_swipe_gesture_threshold(1.0);

  std::fstream pro_config_file_stream;
  pro_config_file_stream.open("BumpTopPro.config", std::ios::out | std::ios::trunc | std::ios::binary);
  // Check if the file opening failed, and if so, the file doesn't exist, so return false
  if (pro_config_file_stream.fail())
    return false;
  std::string pro_config_file_bytes;
  bool success = pro_config_file.SerializeToString(&pro_config_file_bytes);

  //std::string pro_master_key = "9q;3@l&TzIN#&H,V0J/}G,tahKz\",P;M0m{x}\\JYI7#b?qoRREU*fShto*F3xT[iEb,?s,dm[67E905tZc[hBiCHsNT}ZQdT\\/2dsJdp?-o@lIWB:d?mmN\\TM8T.";  // NOLINT  // PRO
  //RC4Encipher(pro_master_key.c_str(), pro_master_key.size(),
  //            const_cast<char*>(pro_config_file_bytes.data()), pro_config_file_bytes.size());

  pro_config_file_stream << pro_config_file_bytes;
  pro_config_file_stream.close();

  if (success)
    return 0;
  else
    return 1;
  }
