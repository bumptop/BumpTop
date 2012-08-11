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

import xml.parsers.expat

class StatsParser:
    def __init__(self):
        self.stack = []
        self.dict = {}
    
    def start_element(self, name, attrs):
        self.stack.append(name)
        #if len(self.stack) == 3 and self.stack[1] == '_statsData':
        #    self.dict[self.stack[2]] = None

    def end_element(self, name):
        self.stack.pop()

    def char_data(self, data):
        if len(self.stack) == 3 and self.stack[1] == '_statsData':
            try:
                self.dict[self.stack[2]] = int(data)
            except ValueError:
                try:
                    self.dict[self.stack[2]] = float(data)
                except ValueError:
                    self.dict[self.stack[2]] = data
                
                    

    def parse(self, data):

        p = xml.parsers.expat.ParserCreate()

        p.StartElementHandler = self.start_element
        p.EndElementHandler = self.end_element
        p.CharacterDataHandler = self.char_data

        p.Parse(data, 1)
            
        return self.dict
        
if __name__ == "__main__":
    f = open("stats_{08B44B89-A1BE-4896-85D6-6993D0AEEEA4}_3127916786_1212069906.xml")
    s = StatsParser()
    data = f.read()
    print s.parse(data)
    

