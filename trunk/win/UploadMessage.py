from subprocess import call
import ftplib
import os
import sys
import urllib

PYTHON = 'C:\Python25\python.exe '
S3FUNNEL = PYTHON + '"' + os.getcwd() + '\\bin\\s3funnelcmd" download.bumptop.com '

def usage():
    print """
usage:

    UploadMessage.py
    
    upload a file as message.txt:
    > python UploadBumptop.py path/of/message.txt
	
	upload a file as message.txt:
    > python UploadBumptop.py path/of/vip_message.txt
    
    delete message.txt
    > python UploadBumpTop.py delete_message
	
    delete vip_message.txt
    > python UploadBumpTop.py delete_vip_message

    """
	
def main(argv=None):
    if argv is None:
        argv = sys.argv
    if len(argv) != 2:
        usage()
    elif os.path.basename(sys.argv[1]) == 'delete_message':
        call(S3FUNNEL + ' DELETE message.txt')
    elif os.path.basename(sys.argv[1]) == 'delete_vip_message':
        call(S3FUNNEL + ' DELETE vip_message.txt')
    elif os.path.basename(sys.argv[1]) != 'message.txt' and os.path.basename(sys.argv[1]) != 'vip_message.txt':
        print "Error: File must be called message.txt or vip_message.txt"
    else:
        call(S3FUNNEL + ' PUT %s' % sys.argv[1])
		
if __name__ == '__main__':
    main()
   