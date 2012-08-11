# Copyright 2011 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cppfile = file("TexHelperResources.cpp")
cppfilecontents = cppfile.read()
cppfile.close()

start = cppfilecontents.find("static size_t xml_res_size_0")
end = cppfilecontents.rfind("static size_t xml_res_size_3")

cppfilecontents = cppfilecontents[start:end]
cppfilecontents = '#include "BT_Common.h"\n' + cppfilecontents

cppfile = file("TexHelperResources.cpp", 'w')
cppfile.write(cppfilecontents)
cppfile.close()
