#!/usr/bin/python

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

from subprocess import call
import ftplib
import os
import sys
import urllib

PYTHON = 'C:\Python26\python.exe '
S3FUNNEL = PYTHON + '"' + os.getcwd() + '\\bin\\s3funnelcmd" download.bumptop.com '



versions = {}
                          
old_versions = {'old_release' : { 'version_prefix' : None, 'version_download_url' :  'http://download.bumptop.com/'},
                'old_vip' : { 'version_prefix' : 'vip_', 'version_download_url' :  'http://download.bumptop.com/'}}

def handle_status():
    all_versions = {}
    for k, v in versions.iteritems():
        all_versions[k] = v
    for k, v in old_versions.iteritems():
        all_versions[k] = v
    for version in all_versions:
        print '%s: ' % version
        prefix = ""
        if all_versions[version]['version_prefix']: prefix = all_versions[version]['version_prefix']
        for f in prefix + 'version.txt', prefix + 'desc.txt', prefix + 'md5.txt', prefix + 'url.txt':
            url = all_versions[version]['version_download_url'] + f
            a_file = urllib.urlopen(url)
            print '\t%s: %s' % (f, a_file.read())
            a_file.close()

def usage():
    print """
usage:

    UploadBumptop.py
    
    current status:
    > python UploadBumptop.py status
    
    
    upload:
    > python UploadBumpTop.py upload (release|vip) (bt|s3) dir
    (dir should have the following files: version.txt, desc.txt, md5.txt)
    
    
    delete:
    > python UploadBumpTop.py delete (release|vip) (bt|s3)

    """

def upload(baseurl, filename):
    if not os.path.exists(filename):
        print 'file %s does not exist' % filename
        exit(0)
    print 'uploading %s to %s...' % (filename, baseurl)
    if baseurl.startswith('s3://download.bumptop.com'):
        call(S3FUNNEL + ' PUT %s' % filename)
    elif baseurl.startswith('ftp://'):
        username, rest = baseurl[6:].split(':')
        password, rest = rest.split('@')
        
        first_slash = rest.find("/")
        domain, path = rest[0:first_slash], rest[first_slash:]
        
        
        ftp = ftplib.FTP(domain)
        ftp.login(username, password)
        ftp.cwd(path)
        f = file(filename, 'rb')
        ftp.storbinary('STOR '+os.path.basename(filename),f)
        f.close()
        ftp.quit()
    else:
        print 'uploading to %s is not supported' % baseurl
        exit(0)
      
def delete(baseurl, filename):
    if baseurl == 's3://download.bumptop.com':
        call(S3FUNNEL + ' DELETE %s' % filename)
    elif baseurl.startswith('ftp://'):
        username, rest = baseurl[6:].split(':')
        password, rest = rest.split('@')
        
        first_slash = rest.find("/")
        domain, path = rest[0:first_slash], rest[first_slash:]
        
        print username, password, domain, path
        
        ftp = ftplib.FTP(domain)
        ftp.login(username, password)
        ftp.cwd(path)
        ftp.delete(os.path.basename(filename))
        
def get_version_and_host(argv):
    if len(argv) < 4:
        return None, None
    
    version = argv[2]
    host = argv[3]
    print version, host
    if version not in versions.keys():
        return None, None
    if host not in ['bt', 's3']:
        return None, None
    return version, host
        
    
def handle_upload(argv):
    version, host = get_version_and_host(argv)
    if version == None or len(argv) < 5:
        if len(argv) < 5: print "No directory specified"
        usage()
        return
        
    dir = argv[4]
    os.chdir(dir)
    
    # make a url file
    # upload the actual installer itself
    prefix = ""
    if versions[version]['version_prefix']: prefix = versions[version]['version_prefix']
    

    
    for f in prefix + 'version.txt', prefix + 'desc.txt', prefix + 'md5.txt', versions[version]['installername']:
        if not os.path.exists(f):
            print f + ' does not exist'
            exit(0)
    
    urlfile = file(prefix + 'url.txt', 'w')
        
    if host == 's3':
        urlfile.write(versions[version]['s3_download_url'] + versions[version]['installername'])
        urlfile.close()
        upload(versions[version]['s3_upload_url'], versions[version]['installername'])
        # for now, the .exe installer will always be living at bumptop... if we change this, we'd have to change download.php as well
        #upload(versions[version]['bt_upload_url'], versions[version]['installerexename'])
    elif host == 'bt':
        urlfile.write(versions[version]['bt_download_url'] + versions[version]['installername'])
        urlfile.close()
        upload(versions[version]['bt_upload_url'], versions[version]['installername'])
        # for now, the .exe installer will always be living at bumptop... if we change this, we'd have to change download.php as well
        #upload(versions[version]['bt_upload_url'], versions[version]['installerexename'])

    # upload the version, desc, url files
    for f in prefix + 'version.txt', prefix + 'desc.txt', prefix + 'md5.txt', prefix + 'url.txt':
        upload(versions[version]['version_upload_url'], f)

    
def handle_delete(argv):
    version, host = get_version_and_host(argv)
    if version == None:
        usage()
        return
    
    if host == 's3':
        delete(versions[version]['s3_upload_url'], versions[version]['installername'])
    elif host == 'bt':
        delete(versions[version]['bt_upload_url'], versions[version]['installername'])
    

        
def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    if len(argv) < 2:
        usage()
    else:
        if argv[1] == 'status':
            handle_status()
        elif argv[1] == 'upload':
            handle_upload(argv)
        elif argv[1] == 'delete':
            handle_delete(argv)
        else:
            usage()
   
    
    

    
if __name__ == '__main__':
    main()
    
