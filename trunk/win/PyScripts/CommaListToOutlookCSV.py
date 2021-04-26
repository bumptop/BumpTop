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

#Put a comma-separated list of emails into the 'vips' variable and this will split out a CSV.
#From there you can import it into Gmail and ask it add it to the VIP list for example. 


vips='''mrcai7@gmail.com, gal.bumptop@szkolnik.com, angryfrommanchester@cityblue.co.uk, qweeeee@mail4trash.com, bill.t.liu@gmail.com, zagar@aixdesign.net, a.robinson91@gmail.com, jongscripter@gmail.com, dmondark@gmail.com, Oberlin_M@hotmail.com, jmrlopez@gmail.com, pikachu098@hotmail.com, thedragoninsideme@hotmail.com, zamadatix@gmail.com, gal.bumptop@szkolnik.com, jzraikes@googlemail.com, hlynurhalldors@gmail.com, surfinsilky@hotmail.com, morningmonster@gmail.com, fidel_1992@hotmail.de, YunoIRC@gmail.com,mikejurka@gmail.com, wty@inbox.lv, jontyab@gmail.com, pcstalljr16@yahoo.com, shreddedcheddar@gmail.com, sajjaad_ramzeydss@yahoo.com, apgriffith@comcast.net, t.hianik@gmail.com, kfqd@bellsouth.net, aaaaaaaaaaaaaaaaaaaaaaaaaaa@master.com, ammok1972@yahoo.fr, romantz@nana10.co.il'''
vips=vips.replace(' ','').split(',')

#header
print '''"Title","First Name","Middle Name","Last Name","Suffix","Company","Department","Job Title","Business Street","Business Street 2","Business Street 3","Business City","Business State","Business Postal Code","Business Country/Region","Home Street","Home Street 2","Home Street 3","Home City","Home State","Home Postal Code","Home Country/Region","Other Street","Other Street 2","Other Street 3","Other City","Other State","Other Postal Code","Other Country/Region","Assistant's Phone","Business Fax","Business Phone","Business Phone 2","Callback","Car Phone","Company Main Phone","Home Fax","Home Phone","Home Phone 2","ISDN","Mobile Phone","Other Fax","Other Phone","Pager","Primary Phone","Radio Phone","TTY/TDD Phone","Telex","Account","Anniversary","Assistant's Name","Billing Information","Birthday","Business Address PO Box","Categories","Children","Directory Server","E-mail Address","E-mail Type","E-mail Display Name","E-mail 2 Address","E-mail 2 Type","E-mail 2 Display Name","E-mail 3 Address","E-mail 3 Type","E-mail 3 Display Name","Gender","Government ID Number","Hobby","Home Address PO Box","Initials","Internet Free Busy","Keywords","Language","Location","Manager's Name","Mileage","Notes","Office Location","Organizational ID Number","Other Address PO Box","Priority","Private","Profession","Referred By","Sensitivity","Spouse","User 1","User 2","User 3","User 4","Web Page"'''

for v in vips:
    print '"","","","","","","","",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,"","0/0/00",,,"0/0/00",,,,,"' + v + '","SMTP","' + v + '",,,,,,,"Unspecified",,,,"",,"","","",,,"'
    print '",,,,"Normal","False",,,"Normal"'
