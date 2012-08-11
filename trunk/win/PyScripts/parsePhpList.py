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

import sys
import os, os.path
import re

# 
emailRegex = re.compile('[^@]*@(.*)')

#

def getDomain(email):
	domain = ''
	match = emailRegex.match(email)
	if match:
		domain = match.group(1).strip().lower()
	return domain
	
def correctSpelling(domain):
	corrections = { "gmai.com":"gmail.com", 
					"gmal.com":"gmail.com", 
					"gmial.com":"gmail.com", 
					"yhoo.com" : "yahoo.com",
					"yaho.com" : "yahoo.com",
					"hotmal.com" : "hotmail.com", 
					"hotmai.com" : "hotmail.com", 
					"htomail.com" : "hotmail.com",
					"hotamail.com" : "hotmail.com",
					"hotmail.om" : "hotmail.com", 
					"otmail.com" : "hotmail.com" }
	return corrections[domain] if corrections.has_key(domain) else None
					

		
	
class PHPListEntry:    
    def __init__(self, tokenizedLine):
        self.id = tokenizedLine[0]
        self.email = tokenizedLine[1]
        self.emailRoot = getDomain(self.email)
        c = correctSpelling(self.emailRoot)
        if c:
            self.email = self.email.replace(self.emailRoot, c)
            self.emailRoot = c
        self.enteredDate = tokenizedLine[2]
        self.modifiedDate = tokenizedLine[3]
        self.htmlEmails = tokenizedLine[4]
        self.whichPage = tokenizedLine[5]
        self.rssFrequency = tokenizedLine[6]
        self.password = tokenizedLine[7]
        self.disabled = tokenizedLine[8]
        self.additionalData = tokenizedLine[9]
        self.foreignKey = tokenizedLine[10]
        self.referrer = tokenizedLine[11]
        self.listMembership = tokenizedLine[12]

    def getKey(self):
        return self.emailRoot

#
def PHPListEntriesByEmailRootList_cmp(l1, l2):
    return cmp(len(l2), len(l1))


# main entry point
if __name__ == "__main__":
    # ensure valid args
    if len(sys.argv) != 2:
        print '(Error) Usage: python %s <PHPList CSV file>' % sys.argv[0]
        exit(1)

    # ensure the csv file exists
    phpListFilename = sys.argv[1]
    if not os.path.exists(phpListFilename):
        print '(Error) File not found at location: %s' % phpListFilename
        exit(1)

    # read the csv file and grab all of the emails
    prevLen = 0
    phpListFile = open(phpListFilename, 'r')
    phpListEntries = []
    for line in phpListFile:
        # tokenize each line
        phpListEntries.append(PHPListEntry(line.split('\t')))

    # enter them into the hashed set of emails if necessary
    phpListEntriesByEmailRoot = {}
    for entry in phpListEntries:
        if entry.getKey() in phpListEntriesByEmailRoot:
            phpListEntriesByEmailRoot[entry.getKey()].append(entry)
        else:
            phpListEntriesByEmailRoot[entry.getKey()] = [entry]

    # iterate the hashed set
    phpListEntriesByEmailRootList = []
    for key in phpListEntriesByEmailRoot:
        phpListEntriesByEmailRootList.append(phpListEntriesByEmailRoot[key])
    phpListEntriesByEmailRootList.sort(PHPListEntriesByEmailRootList_cmp)
    
    print '''<html>
                <head>
                    <!--<LINK rel="stylesheet" href="orderedPhpList.css" type="text/css">-->
                    <style type="text/css">
                    *
                    {
                        font-family: Calibri, Tahoma;
                    }

                    a
                    {
                        color: black;
                        text-decoration: none;
                    }

                    .emailRoot
                    {
                        font-size: 3.0em;
                        font-weight: bold;
                    }

                    .email
                    {
                        font-style: italic;
                        margin-left: 40px;
                    }
                    </style>
                </head>
            <body>'''

    print '<div class="emailRoot">Email Sources (click to see actual emails)</div><br/>'
    for list in phpListEntriesByEmailRootList:
        print '<a href="#%s" class="emailRootToc">%s (%d)</a><br/>' % (list[0].emailRoot, list[0].emailRoot, len(list))
    print '<hr/>'
    for list in phpListEntriesByEmailRootList:
        print '<a name="%s" class="emailRoot">&gt; %s (%d)</a><br/>' % (list[0].emailRoot, list[0].emailRoot, len(list))
        for entry in list:
            print '<div class="email">\t%s</div>' % (entry.email)


    print '''</body>'''        
            
    print 'Complete'
