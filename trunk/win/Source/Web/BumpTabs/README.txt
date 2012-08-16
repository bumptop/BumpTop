Instructions to install Firefox extension for BumpTabs:

1.	Copy the bumptabs@bumptop.com file in this folder to %APPDATA%\Mozilla\Firefox\Profiles\<something>.default\extensions\ .  This is just a plain-text file that can be opened in Notepad.  It should contain the path where BumpTabs is installed - which is by default, C:\Program Files (x86)\BumpTop\Web\BumpTabs

2.	Restart Firefox and you should see a BumpTop icon in your status bar. 

3.	If you used the OpenGL version: In Firefox’s address bar, type “about:config” (without quotes) search for "bumptop" and change the value of extensions.BumpTabs.BumpTopExePath to “C:\Program Files (x86)\BumpTop\BumpTop-OpenGL.exe”

4.	If you are just using SVN source/you did not use an installer:  You will receive an error that BumpTop.exe cannot be found when you attempt to use BumpTabs.  To resolve this, in the Firefox address bar, type “about:config” (without quotes), search for "bumptop" and change the value of extensions.BumpTabs.BumpTopExePath to point to the correct directory/executable.