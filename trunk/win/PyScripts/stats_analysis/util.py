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

def insert_into_sorted_list(new_element, list, less_than_fn = lambda x, y: x < y):
    insert_pos = 0
    for i, element in enumerate(list):
        if less_than_fn(new_element, element): break
        insert_pos = i
    list.insert(insert_pos, new_element)	
    
    prev_item = None
    for item in list:
        if prev_item: assert(less_than_fn(prev_item, item))
                            
def find(f, seq):
  """Return first item in sequence where f(item) == True."""
  for item in seq:
    if f(item): 
      return item
      
def all(S):
    "return true if all the items in sequence are true"
    for x in S:
        if not x:
            return False
    return True
    
def isnumeric(n):
    return isinstance(n, int) or isinstance(n, float) or isinstance(n, long) or isinstance(n, complex)

def reversed(x):
    if hasattr(x, 'keys'):
        raise ValueError("mappings do not support reverse iteration")
    i = len(x)
    while i > 0:
        i -= 1
        yield x[i]
        
def intersection(list1, list2):
    int_dict = {}
    list1_dict = {}
    for e in list1:
        list1_dict[e] = 1
    for e in list2:
        if list1_dict.has_key(e):
            int_dict[e] = 1
    return int_dict.keys()
