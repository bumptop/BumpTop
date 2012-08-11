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

#notes: this script generates a list of users to invite to
#invite.txt is a CSV from mysql of users that are in our database
#the main csv's
from parsePhpList import *

## create a hash table with all the email addresses
def GetMailingListUsers(phpListFilename):
	phpListFile = open(phpListFilename, 'r')
	phpListEntries = []
	for line in phpListFile:
		# tokenize each line
		phpListEntries.append(PHPListEntry(line.split('\t')))
	
	
	email_to_phpListEntry = {}
	for entry in phpListEntries:
		email_to_phpListEntry[entry.email] = entry
		
	
		
	return email_to_phpListEntry



def GetInvitedUsers():
	
	#invited_users_files = [f for f in os.listdir('.') if f[0:7] == 'invited']
	#invited_users_files = ['invited.csv']
	
	invited_users_file = open("invited.csv")
	
	invited_users = []
	
	for line in invited_users_file:
		line = line.split(";")
		if len(line) > 2:
			invited_users.append(line[1][1:-1])
		

	#emailRegex2 = re.compile('([a-zA-Z0-9._-]+@([a-zA-Z0-9-]+\.)+[a-zA-Z.]{2,5})')
	#comma_or_newline = re.compile(',|\n|;')
	
	

	#for f in invited_users_files:
	#	file = open(f, 'r')
	#	entries = comma_or_newline.split(file.read())
	#	matches = [emailRegex2.search(entry) for entry in entries]
	#	invited_users.extend([m.group(0) for m in matches if m])
			
	return invited_users
	
	
if __name__ == "__main__":
	# ensure valid args
	if len(sys.argv) != 3:
		print '(Error) Usage: python %s <PHPList CSV file> <num_invites>' % sys.argv[0]
		exit(1)

	phpList_users = GetMailingListUsers(sys.argv[1])
	
	num_invites = int(sys.argv[2])
	
	invited_users = GetInvitedUsers()
	
	
	# delete already invited users
	for user in invited_users:
		if phpList_users.has_key(user): del phpList_users[user]
	
	
	# delete people from certain companies
	for key, value in phpList_users.items():
		
		if value.emailRoot in ['cs.nyu.edu', 'samsung.com', 'microsoft.com', 'hp.com', 
								'apple.com', 'toshiba.com', 'dell.com', 'google.com', 
								'intel.com', 'yahoo-inc.com', 'goowy.com', 'attglobal.net', 'perceptivepixel.com']:
			del phpList_users[key]

	mod = max(1, len(phpList_users.keys()) / num_invites)
	to_invite = phpList_users.keys()[-num_invites:]#[email for i, email in enumerate(phpList_users.keys()) if i % mod == 0]
	
	print "Showing", len(to_invite), "emails"
	
	for email in to_invite:
		print email
