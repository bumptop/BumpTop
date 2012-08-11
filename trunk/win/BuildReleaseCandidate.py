## Automatically build BumpTop

## Build all the necessary projects first
## Using vcbuild.exe since it allows you to specify a platform
import sys
sys.path.append("bin")

import getopt
import hashlib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time

from add_version_string_to_exe import *


def LOG(str, logToScreen=True, logToFile=True):
	global BUILD_LOG_FILE, VERBOSE
	if logToScreen or VERBOSE:
		print str
	if logToFile:
		BUILD_LOG_FILE.write(str + "\n")

def END_LOG():
	global BUILD_LOG_FILE
	BUILD_LOG_FILE.close()

def getSVNRevision():
	global SVNWCREV
	svnTemplateFile = tempfile.NamedTemporaryFile(delete=False)
	svnTemplateFile.write('$WCREV$')
	svnTemplateFile.close()
	svnArgs = '"%s" . "%s" "%s"' % (SVNWCREV, svnTemplateFile.name, svnTemplateFile.name)
	svnProc = subprocess.Popen(svnArgs, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	out = svnProc.stdout.read()	
	tmp_file = open(svnTemplateFile.name, 'r')
	svnRev = int(tmp_file.read())
	tmp_file.close()	
	os.remove(svnTemplateFile.name)
	LOG("SVN Revision: %s" % svnRev)
	return svnRev

# defines
INNOSETUP = "C:\\Program Files\\Inno Setup 5\\iscc.exe"
INNOSETUPx64 = "C:\\Program Files (x86)\\Inno Setup 5\\iscc.exe"
if not os.path.exists(INNOSETUP): 
    INNOSETUP = INNOSETUPx64

SevenZIP = os.path.join(os.getcwd(), "bin\\7z.exe")
MSBUILD = "C:\\Windows\\Microsoft.NET\\Framework\\v3.5\\MSBuild.exe"

DEVENV = "C:\\Program Files\\Microsoft Visual Studio 9.0\\Common7\\IDE\\devenv.exe"
if not os.path.exists(DEVENV):
    DEVENV = "C:\\Program Files (x86)\\Microsoft Visual Studio 9.0\\Common7\\IDE\\devenv.exe"

SVNWCREV = "C:\\Program Files\\TortoiseSVN\\bin\\SubWCRev.exe"
if not os.path.exists(SVNWCREV): 
    SVNWCREV = "C:\\Program Files (x86)\\TortoiseSVN\\bin\\SubWCRev.exe"

SOLUTION = os.path.join(os.getcwd(), "Source\\BumpTop.sln")

BUILD_LOG_FILENAME = "build.log"
BUILD_LOG_PATH = os.path.join(os.getcwd(), BUILD_LOG_FILENAME)
BUILD_LOG_FILE = open(BUILD_LOG_PATH, 'w+')
VERBOSE = False
VERSIONS = []
RELEASE_EXE = os.path.join(os.getcwd(), "Source\\Release\\BumpTop.exe")
RELEASE_EXE_DXR = os.path.join(os.getcwd(), "Source\\Release\\BumpTop.exe")
TESTING_LOG_FILENAME = "Source\\Tests\\test_log.txt"
TESTING_FLAGS = "-skipRendering -automatedJSONTesting"

LOG("====================")
LOG("BUILDING:")
svnRev = getSVNRevision()
LOG("Average buildtime: 5 minutes")
LOG("====================")

RESOURCE_FILENAME = "Source\\InputDialog.rc"
BACKUP_RESOURCE_FILENAME = "Source\\InputDialog.rc.bak"
SOURCE_FILE_FILTER = ".*\.(inl|cpp|h)$" #copy only these when copying source
RELEASE_IGNORE_FILE_FILTER = ".*\.(svn)$" #ignore these when copying release files
MAJOR_VERSION = 2
MINOR_VERSION = 5
VARIATION = "v3"
STUBversion = "stub7z_v1"

RELEASE_INSTALLER = "BumpTop Inno Installer\\BumpTop-%d.%d-%d.exe" % (MAJOR_VERSION, MINOR_VERSION, svnRev)
STUB_INSTALLER = "BumpTop Inno Installer\\BumpTopStubInstaller.exe"
VIP_INSTALLER = "BumpTopVIP-%d.%d-%d.exe" % (MAJOR_VERSION, MINOR_VERSION, svnRev)
VIP_STUB_INSTALLER = "BumpTopStubInstallerVIP.exe"

signCertSHA1 = "3BFF125D720E9C074B69811EAD3B0A164F633953"
#note lack of e in timstamp.dll below
TIMESERVER = "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL = "bin\\signtool.exe"
SIGN_FILE_FILTER = ".*\.(exe|dll)$"

GREP_BIN = "bin\\grep.exe"
MT_BIN = "mt.exe" # http://msdn.microsoft.com/en-us/library/aa375649(VS.85,classic).aspx
MANIFEST_FILTER = ".*.(exe|dll)$"


oldCRTs = [
	"8.0.50727.762", 
	"8.0.50727.891", 
	"8.0.50727.816", 
	"8.0.50727.42", 
	"3ca5156e8212449db6c622c3d10f37d9adb12c66",
	"8.0.50727.4053",
	"9.0.30729.1",
	"9.0.30729.17",
]
newCRTs = [
	"9.0.30729.4148"
]

# IMPORTANT: 7z verification tests number of file in archive.
# If files are added/removed from installer, this value MUST be
# updated to match.  This will ensure errant files do not sneak
# into the installer.
# NOTE: this needs to be enhanced to deal with various build configurations
# including OEM build configurations
# sevenZIPNUMFILES = "Files: 191"
# buildDir = os.path.join(os.getcwd(), buildFormat % (svnRev, version, MAJOR_VERSION, MINOR_VERSION, svnRev))

INNOSETUPSCRIPTPREPROCESS = {
	".*" : 		
				[	
						("AppVersion=0.0.1337", ("AppVersion=%d.%d.%d\n" % (MAJOR_VERSION, MINOR_VERSION, svnRev))),
						("VersionInfoVersion=0.0.1337", ("VersionInfoVersion=%d.%d.%d\n" % (MAJOR_VERSION, MINOR_VERSION, svnRev))),
						("OutputBaseFilename=BumpTop", ("OutputBaseFilename=BumpTop-%d.%d-%d\n" % (MAJOR_VERSION, MINOR_VERSION, svnRev)))
				],
	"specificDummyPattern.iss" : 
				[	
						("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_jw.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")
				]
}
# multiple preprocessor defines should be separated by a space in the same string
REQUIRES_CUSTOM_VERSION = {
	"OEM_INTEL_ATOM" : {
		"DEFINES" : "ENABLE_ADP_WRAPPER"
	}
}
INNOSETUPSCRIPT = {
	"Release" : "BumpTop Inno Installer\\Installer.iss",
	"ReleaseDX" : "BumpTop Inno Installer\\Installer.iss",
	"VIP" : "BumpTop Inno Installer\\Installer.iss",
	"OEM_ACER" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"OEM_ASUS" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"OEM_HP" : "BumpTop Inno Installer\\Installer_OEM_HP.iss",
	"OEM_JW" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"OEM_LENOVO" : "BumpTop Inno Installer\\Installer_OEM_Lenovo.iss",
	"OEM_MSI" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"OEM_INTEL_ATOM" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"VGA_ASUS" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"VGA_COLORFUL" : "BumpTop Inno Installer\\Installer_VGA_Colorful.iss",
	"VGA_HIS" : "BumpTop Inno Installer\\Installer_VGA_HIS.iss",
	"VGA_SAPPHIRE" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"VGA_TUL" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"VGA_INPAI" : "BumpTop Inno Installer\\Installer_OEM.iss",
	"TRADESHOW" : "BumpTop Inno Installer\\Installer_Tradeshow.iss",
	"TRADESHOW_SAPPHIRE" : "BumpTop Inno Installer\\Installer_Tradeshow.iss",
}
INNOSETUPSTUBSCRIPT = {
	"Release" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"ReleaseDX" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VIP" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_ACER" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_ASUS" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_HP" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_JW" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_LENOVO" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_MSI" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"OEM_INTEL_ATOM" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_ASUS" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_COLORFUL" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_HIS" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_SAPPHIRE" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_TUL" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"VGA_INPAI" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"TRADESHOW" : "BumpTop Inno Installer\\Installer_Stub.iss",
	"TRADESHOW_SAPPHIRE" : "BumpTop Inno Installer\\Installer_Stub.iss"
}
TARGETS = [
	("BumpTop", "Win32"),
	("BumpTop Settings", "Win32"),
	("GetIconPositions", "x64"),
	("PostExec", "Win32"),
	("TexHelper", "Win32")
]
RELEASE_LOOSE_FILES = [
	"Source\\Release\\BumpTop.exe",
	"Source\\Release\\BumpTop Settings.exe",
	"Source\\Release\\TexHelper.exe",
	"Source\\Release\\PostExec.exe",
	"Source\\Release\\DevIL.dll",
	"Source\\Release\\AdpWrapper.dll",
	"Source\\Release\\D3DX9_41.dll",
	"Source\\Release\\glew32.dll",
	"Source\\Release\\ILU.dll",
	"Source\\Release\\ILUT.dll",
	"Source\\Release\\NxFoundation.dll",
	"Source\\Release\\NxPhysics.dll",
	"Source\\Release\\QtCore4.dll",
	# only used for Loose/debugging, these following files are explicitly removed from 7z/stub
	"Source\\Release\\BumpTop.pdb",
	#"Source\\Release\\BumpTopInstaller.msi",
	# new added in v1.5
	"Source\\Release\\QtWebKit4.dll",
	"Source\\Release\\QtNetwork4.dll",
	"Source\\Release\\QtGui4.dll",
	# added in 2.1 for qt 4.6.1
	"Source\\Release\\phonon4.dll",
	"Source\\Release\\QtXmlPatterns4.dll",
	# added back in 2.1 for facebook login
	"Source\\Release\\libeay32.dll",
	"Source\\Release\\ssleay32.dll"
]
RELEASE_LOOSE_FILES_SHARING = [
	# below added in Ossington for sharing
	"Source\\Sharing\\BumpTop.js",
	"Source\\Sharing\\sharedFolder.html",
	"Source\\Sharing\\sharedFolder.js",
	"Source\\Sharing\\jquery",
	"Source\\Sharing\\images",
	"Source\\Sharing\\dialogs"
]
RELEASE_LOOSE_FILES_WEB = [
	"Source\\Web"
]
RELEASE_LOOSE_FILES_FACEBOOK = [
	# below added in Ossington for Facebook
	"Source\\Facebook"
]
RELEASE_LOOSE_FILES_IMAGEFORMATS = [
	"Source\\Release\\imageformats\\qgif4.dll",
	"Source\\Release\\imageformats\\qico4.dll",
	"Source\\Release\\imageformats\\qjpeg4.dll",
	"Source\\Release\\imageformats\\qmng4.dll",
	"Source\\Release\\imageformats\\qsvg4.dll",
	"Source\\Release\\imageformats\\qtiff4.dll"
]
RELEASE_LOOSE_FILES_X64 = [
	"Source\\Release\\x64\\GetIconPositions.exe",
	"Source\\Release\\x64\\QtCore4.dll"
]
RELEASE_LOOSE_FILES_THEMES = [
	"Source\\Themes\\theme.schemas",
	"Source\\Themes\\Default",
	"Source\\Themes\\Bumped Next",
	"Source\\Themes\\BumpTop Classic",
	"Source\\Themes\\Bump Blue"
]
RELEASE_LOOSE_FILES_TEXTURES = [
	"Source\\Textures\\ao_floor_256.png",
	"Source\\Textures\\ao_top_bottom_wall_256.png",
	"Source\\Textures\\bumptop.ico",
	"Source\\Textures\\bumptoplogo-training.png",
	"Source\\Textures\\DSC_0384-training.JPG",
	"Source\\Textures\\DSCF0534-2-training.jpg",
	"Source\\Textures\\emptyPhotoFrame.png",
	"Source\\Textures\\flickr.png",
	"Source\\Textures\\facebook.png",
	"Source\\Textures\\loader.gif",
	"Source\\Textures\\loadingPhotoFrame.png",
	#"Source\\Textures\\printer.png", included in themes instead
	"Source\\Textures\\pui-close.png",
	"Source\\Textures\\pui-divider.png",
	"Source\\Textures\\pui-editor.png",
	"Source\\Textures\\pui-email.png",
	"Source\\Textures\\pui-facebook.png",
	"Source\\Textures\\pui-next.png",
	"Source\\Textures\\pui-previous.png",
	"Source\\Textures\\pui-print.png",
	"Source\\Textures\\pui-twitter.png",
	"Source\\Textures\\removable_disc.png",
	"Source\\Textures\\removable_drive.png",
	"Source\\Textures\\Slide1-training.JPG",
	"Source\\Textures\\Slide2-training.JPG",
	"Source\\Textures\\splash.jpg",
	"Source\\Textures\\stickyNotePad.png",
	"Source\\Textures\\stickyNotePadDisabled.png",
	"Source\\Textures\\square.png",
	"Source\\Textures\\throbber.gif",
	"Source\\Textures\\touch_point.png",
	"Source\\Textures\\twitter.png",
	"Source\\Textures\\winAVI.avi",
	"Source\\Textures\\winFolder.png",
	"Source\\Textures\\winImage4.png",
	"Source\\Textures\\winMOV.png",
	"Source\\Textures\\winNotes.png",
	"Source\\Textures\\winNotes.txt",
	"Source\\Textures\\winPDF2.pdf",
	"Source\\Textures\\winPDF2.png",
	"Source\\Textures\\winWord.doc",
	"Source\\Textures\\winWord.png",
	"Source\\Textures\\zoom_out.png",
	"Source\\Textures\\logo_his.png"
]
RELEASE_LOOSE_FILES_LANGUAGES = [
	"Source\\Languages\\Settings_de.mo",
	"Source\\Languages\\Settings_en.mo",
	"Source\\Languages\\Settings_es.mo",
	"Source\\Languages\\Settings_fr.mo",
	"Source\\Languages\\Settings_it.mo",
	"Source\\Languages\\Settings_ja.mo",
	"Source\\Languages\\Settings_ko.mo",
	"Source\\Languages\\Settings_pt.mo",
	"Source\\Languages\\Settings_ru.mo",
	"Source\\Languages\\Settings_zh_cn.mo",
	"Source\\Languages\\Settings_zh_tw.mo",
	"Source\\Languages\\BumpTop.de.qm",
	"Source\\Languages\\BumpTop.en.qm",
	"Source\\Languages\\BumpTop.es.qm",
	"Source\\Languages\\BumpTop.fr.qm",
	"Source\\Languages\\BumpTop.it.qm",
	"Source\\Languages\\BumpTop.ja.qm",
	"Source\\Languages\\BumpTop.ko.qm",
	"Source\\Languages\\BumpTop.pt.qm",
	"Source\\Languages\\BumpTop.ru.qm",
	"Source\\Languages\\BumpTop.zh_cn.qm",
	"Source\\Languages\\BumpTop.zh_tw.qm"
]
REPLACE_FILES_DEFAULT = [
	("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_original.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg"),
	("Source\\Themes\\Default\\private_override\\theme_original.json", "Source\\Themes\\Default\\theme.json"),
]
REPLACE_FILES = {
	"OEM_JW" : [		("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_jw.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")],
	"VGA_TUL" : [		("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_TUL.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")],
	"VGA_COLORFUL" : [	("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_colorful.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")],
	"VGA_SAPPHIRE" : [	("Source\\Themes\\Default\\private_override\\theme_sapphire.json", "Source\\Themes\\Default\\theme.json"),
						("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_sapphire.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg"),	
						("Source\\Themes\\Default\\desktop\\private_override\\wall_front_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_front_sapphire.jpg"),
						("Source\\Themes\\Default\\desktop\\private_override\\wall_right_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_right_sapphire.jpg"),
						("Source\\Themes\\Default\\desktop\\private_override\\wall_left_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_left_sapphire.jpg") ],
	"VGA_HIS" : [	("Source\\Themes\\Default\\private_override\\theme_his.json", "Source\\Themes\\Default\\theme.json"),
						("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_his.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop_his.jpg"),	
						("Source\\Themes\\Default\\desktop\\private_override\\wall_front_his.jpg", "Source\\Themes\\Default\\desktop\\wall_front_his.jpg"),
						("Source\\Themes\\Default\\desktop\\private_override\\wall_his.jpg", "Source\\Themes\\Default\\desktop\\wall_his.jpg") ],
	"VGA_INPAI" : [	("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_inpai.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")],
	"TRADESHOW" : [		("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_logo.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg")],
	"TRADESHOW_SAPPHIRE" : [ ("Source\\Themes\\Default\\private_override\\theme_sapphire.json", "Source\\Themes\\Default\\theme.json"),
						("Source\\Themes\\Default\\desktop\\private_override\\floor_desktop_sapphire.jpg", "Source\\Themes\\Default\\desktop\\floor_desktop.jpg"),	
						("Source\\Themes\\Default\\desktop\\private_override\\wall_front_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_front_sapphire.jpg"),
						("Source\\Themes\\Default\\desktop\\private_override\\wall_right_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_right_sapphire.jpg"),
						("Source\\Themes\\Default\\desktop\\private_override\\wall_left_sapphire.jpg", "Source\\Themes\\Default\\desktop\\wall_left_sapphire.jpg") ]
}
DELETE_FILES = [
	"Source\\Release\\BumpTop.exe",
	"Source\\Themes\\Default\\desktop\\wall_front_sapphire.jpg",		# clear out the sapphire overrides if any
	"Source\\Themes\\Default\\desktop\\wall_right_sapphire.jpg",
	"Source\\Themes\\Default\\desktop\\wall_left_sapphire.jpg",
	"Source\\Themes\\Default\\desktop\\wall_front_his.jpg",
	"Source\\Themes\\Default\\desktop\\wall_his.jpg"
]
SKIP_ALTER_EXE = [
	"Release"	
]
	
def ensureDependencies():
	"""Return True if all dependencies are met, False otherwise."""
	LOG("Required Dependencies: VS2008 bin, Inno Setup")
	LOG("Optional: Bump code signing certificate imported")

	if not os.path.exists(INNOSETUP) and not os.path.exists(INNOSETUPx64):
		print "Couldn't find Inno Setup in %s or %s." % (INNOSETUP, INNOSETUPx64)
		return False
	if not os.path.exists(SevenZIP):
		print "Couldn't find 7-zip in %s" % (SevenZIP)
		return False
	if not os.path.exists(SVNWCREV):
		print "Couldn't find Tortoise SVN in %s" % (SVNWCREV)
		return False

	return True
	
def cleanProjectOutputs():
	global MSBUILD, SOLUTION
	LOG("Cleaning Project Outputs:")
	# build each of the other project targets that we need
	# XXX
	buildFormats = ['"%s" "%s" /p:Configuration="Release" /t:"clean"']
	# removing OpenGL build configuration     , '"%s" "%s" /p:Configuration="Release_GL" /t:"clean"'
	for buildFormat in buildFormats:
		start = time.time()
		buildPath = buildFormat  % (MSBUILD, SOLUTION)
		buildProc = subprocess.Popen(buildPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
		buildOut = buildProc.stdout.read()
		if buildOut.find("0 Error(s)") < 0:
			LOG("    Failed :(")
			LOG("-------------")
			LOG(buildOut)
			sys.exit(1)
		else:
			LOG("    Succeeded [%ds]" % (time.time() - start))
			
def runTests():
	LOG("  Starting automated tests")
	start = time.time()
	# Invoke the tests
	subprocess.Popen([RELEASE_EXE, "-automatedJSONTesting"]).communicate()

	# Examine the results
	retcode = 1
	completed = False
	with open(TESTING_LOG_FILENAME) as f:
		for line in f.readlines():
			if line.startswith("TESTING COMPLETE"):
				completed = True
				LOG ("\t%s" % line.strip())
				retcode = 0 if ("0 FAILED" in line) else 1

	if not completed:
		LOG ("ERROR: Tests aborted -- see %s" % TESTING_LOG_FILENAME)
	else:			
		LOG("    Succeeded [%ds]" % (time.time() - start))
		
def buildProjectOutputs(versions):
	global MSBUILD, TARGETS, SOLUTION
	global DELETE_FILES
	global REQUIRES_CUSTOM_VERSION
		
	# ensure that we don't have multiple versions building where one version
	# requires a different project output than another		
	if len(versions) > 1:
		for version in versions:
			if version in REQUIRES_CUSTOM_VERSION:
				LOG("Custom build required for one of the versions you selected: %s" % (version))
				sys.exit(1)	
				
	# determine all the preprocessor defines
	defines = ''
	for version in versions:
		if version in REQUIRES_CUSTOM_VERSION:
			defines = REQUIRES_CUSTOM_VERSION[version]["DEFINES"]
	
	# clear any files that we need to first
	LOG("Deleting (Potentially) Conflicting Files:")
	for file in DELETE_FILES:
		deleteFilePath = os.path.join(os.getcwd(), file)		
		if os.path.exists(deleteFilePath):
			LOG("  Deleting %s" % (file))
			os.remove(deleteFilePath)
		
	LOG("Building Project Outputs:")
	# build each of the other project targets that we need
	# XXX
	buildFormats = ['"%s" "%s" /p:Configuration="Release" /t:"%s" /p:Platform="%s" /p:DefineConstants=%s']
	# removing OpenGL build configuration     , '"%s" "%s" /p:Configuration="Release_GL" /t:"%s" /p:Platform="%s" /p:DefineConstants=%s'
	for buildFormat in buildFormats:
		for target in TARGETS:
			LOG("  Building: %s|%s" % (target[0], target[1]))
			start = time.time()
			buildPath = buildFormat  % (MSBUILD, SOLUTION, target[0], target[1], defines)
			buildProc = subprocess.Popen(buildPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
			buildOut = buildProc.stdout.read()
			if buildOut.find("0 Error(s)") < 0:
				LOG("    Failed :(")
				LOG("-------------")
				LOG(buildOut)
				sys.exit(1)
			else:
				LOG("    Succeeded [%ds]" % (time.time() - start))
			LOG("\t--------------------\n%s\n%s" % (buildPath, buildOut), False)
	
def createBuildDirectory(version, svnRev):
	global VARIATION, MAJOR_VERSION, MINOR_VERSION
	count = 1
	buildFormat = "%d - %s - %d.%d.%d"
	buildDir = os.path.join(os.getcwd(), buildFormat % (svnRev, version, MAJOR_VERSION, MINOR_VERSION, svnRev))
	while os.path.exists(buildDir):
		buildDir = os.path.join(os.getcwd(), buildFormat % (svnRev, version, MAJOR_VERSION, MINOR_VERSION, svnRev))
		buildDir = "%s_%d" % (buildDir, count)
		count += 1
	os.mkdir(buildDir)

	# write the svn revision file
	if version == "Release":
		simpleWriteFile(os.path.join(buildDir, "%s_version.txt" % VARIATION), str(svnRev))
	elif version == "VIP":
		simpleWriteFile(os.path.join(buildDir, "%s_%s_version.txt" % (version.lower(), VARIATION)), str(svnRev))
	else:
		simpleWriteFile(os.path.join(buildDir, "%s_version.txt" % (version)), str(svnRev))
	
	LOG("\tCreating directory:\n  %s" % buildDir)
	return buildDir
	
def validateReleaseVersion(buildDir, version):
	return
	global SKIP_ALTER_EXE
	if version in SKIP_ALTER_EXE:
		# the exe was not altered, so we don't need to validate it
		return
		
	global RELEASE_EXE
	# validate this is correct
	validatePath = '"%s" -dumpVersionInfo' % RELEASE_EXE
	validateProc = subprocess.Popen(validatePath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	validateOut = validateProc.stdout.read()
	reportedVersionStr = simpleReadFile("BUMPTOP_VERSION.txt")
	if reportedVersionStr.find(version) < 0:
		LOG("Invalid release exe, should report %s but contains %s" % (version, reportedVersionStr))
		sys.exit(1)
	else:
		LOG("BumpTop.exe altered to version %s" % version)
		shutil.move("BUMPTOP_VERSION.txt", os.path.join(buildDir, "exe_version.txt"))
		
def alterResourceVersion(svnRev):
	global RESOURCE_FILENAME, BACKUP_RESOURCE_FILENAME, MAJOR_VERSION, MINOR_VERSION
	
	# save a backup of the resource file
	shutil.copy2(os.path.join(os.getcwd(), RESOURCE_FILENAME), os.path.join(os.getcwd(), BACKUP_RESOURCE_FILENAME))
	
	# read the file line by line
	lines = open(RESOURCE_FILENAME, 'r').readlines()
	fileVersionRegex 		= re.compile(r'.*FILEVERSION.*')
	productVersionRegex 	= re.compile(r'.*PRODUCTVERSION.*')
	fileVersionAltRegex 	= re.compile(r'\s*VALUE "FileVersion"')
	productVersionAltRegex 	= re.compile(r'\s*VALUE "ProductVersion"')
	file = open(RESOURCE_FILENAME, 'w+')
	for line in lines:		
		if fileVersionRegex.match(line):
			line = ' FILEVERSION %d,%d,%d,0\n' % (MAJOR_VERSION, MINOR_VERSION, svnRev)
		elif productVersionRegex.match(line):
			line = ' PRODUCTVERSION %d,%d,%d,0\n' % (MAJOR_VERSION, MINOR_VERSION, svnRev)
		elif fileVersionAltRegex.match(line):
			line = '            VALUE "FileVersion", "%d, %d, %d, 0"\n' % (MAJOR_VERSION, MINOR_VERSION, svnRev)
		elif productVersionAltRegex.match(line):
			line = '            VALUE "ProductVersion", "%d.%d.%d"\n' % (MAJOR_VERSION, MINOR_VERSION, svnRev)
		file.write(line)
	file.close()
		
def alterReleaseFiles(buildDir, version):
	global REPLACE_FILES_DEFAULT, REPLACE_FILES
	
	# restore the original files first
	for file in REPLACE_FILES_DEFAULT:		
		LOG("  Replacing %s with %s" % (os.path.basename(file[1]), os.path.basename(file[0])))
		shutil.copy2(os.path.join(os.getcwd(), file[0]), os.path.join(os.getcwd(), file[1]))
		
	# alter the release specific files
	if version in REPLACE_FILES:
		alterFilesList = REPLACE_FILES[version]
		for file in alterFilesList:
			LOG("  Replacing %s with %s" % (os.path.basename(file[1]), os.path.basename(file[0])))
			shutil.copy2(os.path.join(os.getcwd(), file[0]), os.path.join(os.getcwd(), file[1]))
	
def alterReleaseVersion(buildDir, version):
	global SKIP_ALTER_EXE
	if version in SKIP_ALTER_EXE:
		# don't alter the exe, just the files (if any)
		alterReleaseFiles(buildDir, version)
		return;
		
	global RELEASE_EXE
	LOG("Altering release files:")
	LOG("  Adding resource %s to %s" % (version, os.path.basename(RELEASE_EXE)))
	add_version_string_to_exe(os.path.join(os.getcwd(), "bin"), RELEASE_EXE, version)
	# validate the exe after altering it
	validateReleaseVersion(buildDir, version)
	alterReleaseFiles(buildDir, version)

def generateMD5s(dir):
	LOG("Generating MD5s in directory %s" % dir)
	targetFiles = os.listdir(dir)
	targetFilesRegex = re.compile(".*\.(exe)$")

	for file in targetFiles:
		LOG("\tfilename: %s" % file)
		if targetFilesRegex.match(file, re.IGNORECASE):
			LOG("\tmatched filename: %s" % file)
			relFilePath = os.path.join(dir, file)
			filePath = os.path.join(os.getcwd(), relFilePath)
			
			md5FileName = ("%s_md5.txt" % (file[0:-4]))
			md5FilePath = os.path.join(dir, md5FileName)
			LOG("\ttrying to write to: %s" % md5FileName)
			md5FileHash = generateMD5(filePath)
			simpleWriteFile(md5FilePath, md5FileHash)
	
def generateMD5(filePath):	
	file = open(filePath, 'rb')
	md5 = hashlib.md5()
	md5.update(file.read())
	file.close()
	return md5.hexdigest()

def simpleWriteFile(filePath, content):
	file = open(filePath, 'w+')
	file.write(content)
	file.close()
	
def simpleReadFile(filePath):
	file = open(filePath, 'r')
	content = file.read()
	file.close()
	return content

def preprocessFiles():
	# process the installer script and prepare for building
	LOG("Preprocessing Inno Setup Installer")
	start = time.time()
	innoSetupFileName = INNOSETUPSCRIPT[version]
	for replaceFileEntry in INNOSETUPSCRIPTPREPROCESS:
		#matcherString = "r'%s'" % replaceFileEntry
		matcherString = r'%s' % replaceFileEntry
		#LOG("      --> %s" % matcherString) # *.iss$
		fileNameMatcherRegex = re.compile(matcherString)
		#fileNameMatcherRegex = re.compile(r'.*')
		if fileNameMatcherRegex.match(innoSetupFileName):
			# this file matches the filter
			# save a backup of the file
			# this technically does not create multiple backups - might be useful in the future for debugging purposes
			# ensure postprocessFiles() is updated if this procedure is changed
			shutil.copy2(os.path.join(os.getcwd(), innoSetupFileName), os.path.join(os.getcwd(), ("%s.preprocessBAK" % innoSetupFileName)))
			for replaceLineEntry in INNOSETUPSCRIPTPREPROCESS[replaceFileEntry]:
				# look for replaceLineEntry[0] and replace with replaceLineEntry[1]
				lineMatcherString = r'%s' % replaceLineEntry[0]
				lineEntryRegex = re.compile(lineMatcherString)
				# load current file
				lines = open(innoSetupFileName, 'r').readlines()
				# overwrite current file
				file = open(innoSetupFileName, 'w+')
				# scan each line of the file
				for line in lines:
					if lineEntryRegex.match(line):
						#line needs to be replaced
						LOG("    PREPROCESS: %s" % innoSetupFileName)
						LOG(("      Replacing %s with %s" % (line, replaceLineEntry[1])))
						line = replaceLineEntry[1]
					# write line to new file
					file.write(line)
				file.close()
	LOG("  Succeeded [%ds]" % (time.time() - start))
				
def postprocessFiles():
	LOG("Postprocessing Inno Setup Installer")
	start = time.time()
	innoSetupFileName = INNOSETUPSCRIPT[version]
	for replaceFileEntry in INNOSETUPSCRIPTPREPROCESS:
		#matcherString = "r'%s'" % replaceFileEntry
		matcherString = r'%s' % replaceFileEntry
		#LOG("      --> %s" % matcherString) # *.iss$
		fileNameMatcherRegex = re.compile(matcherString)
		#fileNameMatcherRegex = re.compile(r'.*')
		if fileNameMatcherRegex.match(innoSetupFileName):
			# this file matches the filter
			# restore backup of the file
			shutil.copy2(os.path.join(os.getcwd(), ("%s.preprocessBAK" % innoSetupFileName)), os.path.join(os.getcwd(), innoSetupFileName))
	LOG("  Succeeded [%ds]" % (time.time() - start))
	
def compressSevenZip(workingDir, parameters, target):
	global SevenZIP
	#global sevenZIPNUMFILES
	root = os.getcwd()
	os.chdir(workingDir)
	sevenZipPath = SevenZIP + parameters % target
	sevenZipOut = ""
	LOG("  Building 7-Zip archive %s" % target)
	start = time.time()
	sevenZipProc = subprocess.Popen(sevenZipPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	sevenZipOut = sevenZipProc.stdout.read()
	if sevenZipOut.find("Everything is Ok") < 0:
		LOG("ERROR: 7z archive generation failed:")
		LOG(sevenZipOut)
		LOG("ERROR: 7z archive generation failed:")
		sys.exit(1)
	else:
		LOG("    Succeeded [%ds]" % (time.time() - start))
	
	# confirm created 7z is valid
	sevenZipPath = SevenZIP + (' t "%s"' % (target))
	sevenZipOut = ""
	LOG("  Validating 7z archive")
	start = time.time()
	sevenZipProc = subprocess.Popen(sevenZipPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	sevenZipOut = sevenZipProc.stdout.read()
	# or (sevenZipOut.find(sevenZIPNUMFILES) < 0)
	if ((sevenZipOut.find("Everything is Ok") < 0)):
		LOG("ERROR: 7z archive generation failed:")
		LOG(sevenZipOut)
		LOG("Looked for: " + target)
		LOG("ERROR: 7z archive generation failed, or files missing from archive:")
		sys.exit(1)
	else:
		LOG("    Succeeded [%ds]" % (time.time() - start))
	os.chdir(root)
	
def buildSevenZip(buildDir):
	global STUBversion, SevenZIP
	#global sevenZIPNUMFILES
	
	# Capture SVN revision number
	SevenZipFileName = "%s.7z" % getSVNRevision()

	# exclude .svn and tmp, max compression, solid archive, multi-thread on
	# see http://www.downloadatoz.com/manual/7z/7zip/cmdline/switches/method.htm
	compressSevenZip(os.path.join(buildDir, "Loose\\"), ' a -xr!.svn -xr!tmp -xr!*.pdb -xr!*.db -xr!*.msi -xr!*.psd -mx=9 -ms -mmt=on %s *', "..\\" + SevenZipFileName)
	
	# NOTE (Justin): This check may not be necessary, but putting this in for now
	# NOTE: as mike points out, we should be validating the exe again to ensure
	# that the install process didn't rebuild the release exe that we just validated!
	validateReleaseVersion(buildDir, version)
	
	root = os.getcwd()
	
	# create an md5 of 7zip file
	os.chdir(buildDir)
	md5Hash = generateMD5(SevenZipFileName)
	if version == "Release":
		md5FileName = "%s_md5.txt" % STUBversion 
	elif version == "VIP":
		md5FileName = "%s_%s_md5.txt" % (version.lower(), STUBversion)
	else:
		md5FileName = "%s_md5.txt" % (STUBversion)
	
	md5FilePath = os.path.join(buildDir, md5FileName)
	simpleWriteFile(md5FilePath, md5Hash)
	LOG("  Writing %s to build directory" % md5FileName)
	
	# create the other associated files (desc, url)	
	if version == "Release":
		simpleWriteFile(os.path.join(buildDir, "%s_desc.txt" % STUBversion), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_url.txt" % STUBversion), "")
	elif version == "VIP":	
		simpleWriteFile(os.path.join(buildDir, "%s_%s_desc.txt" % (version.lower(), STUBversion)), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_%s_url.txt" % (version.lower(), STUBversion)), "")
	else:	
		simpleWriteFile(os.path.join(buildDir, "%s_desc.txt" % (STUBversion)), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_url.txt" % (STUBversion)), "")
	os.chdir(root)
	
def buildBumpTopInstaller(buildDir, version):
	global INNOSETUP, INNOSETUPSCRIPT
	global DEVENV, SOLUTION
	global RELEASE_INSTALLER, VIP_INSTALLER, STUB_INSTALLER
	global RELEASE_LOOSE_FILES, RELEASE_LOOSE_FILES_X64, RELEASE_LOOSE_FILES_THEMES, RELEASE_LOOSE_FILES_TEXTURES, RELEASE_LOOSE_FILES_LANGUAGES, RELEASE_LOOSE_FILES_IMAGEFORMATS, RELEASE_LOOSE_FILES_FACEBOOK, RELEASE_LOOSE_FILES_WEB
	global VARIATION
	
	if version not in INNOSETUPSCRIPT:		
		LOG("Expected inno installer file for version: " % version)
		sys.exit(1)
	
	# build the installer
	LOG("Building Inno Setup Installer")
	start = time.time()
	innoSetupPath = '"%s" "%s"' % (INNOSETUP, INNOSETUPSCRIPT[version])
	innoSetupProc = subprocess.Popen(innoSetupPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	innoSetupOut = innoSetupProc.stdout.read()	
	if innoSetupOut.find("Successful compile") < 0:
		LOG("ERROR: Inno Setup failed:")
		LOG(innoSetupOut)
		LOG("ERROR: Inno Setup failed.")
		sys.exit(1)
	else:
		LOG("  Succeeded [%ds]" % (time.time() - start))
	
	time.sleep(1)
	
	# build inno setup STUB installer
	#LOG("Building Inno Setup Stub Installer")
	#start = time.time()
	#innoSetupPath = '"%s" "%s"' % (INNOSETUP, INNOSETUPSTUBSCRIPT[version])
	#innoSetupProc = subprocess.Popen(innoSetupPath, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	#innoSetupOut = innoSetupProc.stdout.read()	
	#if innoSetupOut.find("Successful compile") < 0:
	#	LOG("ERROR: Inno Setup (Stub) failed:")
	#	LOG(innoSetupOut)
	#	LOG("ERROR: Inno Setup (Stub) failed.")
	#	sys.exit(1)
	#else:
	#	LOG("  Succeeded [%ds]" % (time.time() - start))
		
	# NOTE: as mike points out, we should be validating the exe again to ensure
	# that the install process didn't rebuild the release exe that we just validated!
	validateReleaseVersion(buildDir, version)
	LOG("ping: %s %s" % (RELEASE_INSTALLER, VIP_INSTALLER))
		
	# copy over the the installer and the other files into the build dir
	source = [os.path.join(os.getcwd(), RELEASE_INSTALLER)];
	destination = [];

	if version == "VIP":
		destination = [os.path.join(buildDir, VIP_INSTALLER)];
	else:
		destination = [os.path.join(buildDir, os.path.basename(RELEASE_INSTALLER))];
	
	moveFiles(source[0], destination[0], True);
	#moveFiles(source[1], destination[1], True);

	#if os.path.exists(stubFilePath):
	#	LOG("Copying %s to %s" % (STUB_INSTALLER, os.path.basename(buildDir)))
	#	shutil.move(stubFilePath, newStubFilePath)
	#else:
	#	LOG("Error: Could not find %s" % stubFilePath)
	#	sys.exit(1)
		
	# create an md5 of the regular installer		
	#md5Hash = generateMD5(destination[0])
	#if version == "Release":
	#	md5FileName = "%s_md5.txt" % VARIATION 
	#elif version == "VIP":
	#	md5FileName = "%s_%s_md5.txt" % (version.lower(), VARIATION)
	#else:
	#	md5FileName = "%s_md5.txt" % (version)
	#
	#md5FilePath = os.path.join(buildDir, md5FileName)
	#simpleWriteFile(md5FilePath, md5Hash)
	#LOG("Writing %s to build directory" % md5FileName)
	
	# create the other associated files (desc, url)	
	if version == "Release":
		simpleWriteFile(os.path.join(buildDir, "%s_desc.txt" % VARIATION), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_url.txt" % VARIATION), "")
	elif version == "VIP":	
		simpleWriteFile(os.path.join(buildDir, "%s_%s_desc.txt" % (version.lower(), VARIATION)), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_%s_url.txt" % (version.lower(), VARIATION)), "")
	else:	
		simpleWriteFile(os.path.join(buildDir, "%s_desc.txt" % (version)), "")		
		simpleWriteFile(os.path.join(buildDir, "%s_url.txt" % (version)), "")
		

def moveFiles(source, target, dieOnError):
	if os.path.exists(source):
		LOG("\t\tCopying %s to %s" % (source, target), False, True)
		shutil.move(source, target)
	else:
		if (dieOnError):
			sys.exit(1);
		
def copyLoose(buildDir):
	#ignore files that match this filter
	global RELEASE_IGNORE_FILE_FILTER
	releaseFilesIgnoreRegex = re.compile(RELEASE_IGNORE_FILE_FILTER)
	# copy over the loose/residual files
	dstFolders = [
		("Loose", RELEASE_LOOSE_FILES), 
		("Loose\\x64", RELEASE_LOOSE_FILES_X64), 
		("Loose\\Textures", RELEASE_LOOSE_FILES_TEXTURES), 
		("Loose\\Themes", RELEASE_LOOSE_FILES_THEMES),
		("Loose\\Languages", RELEASE_LOOSE_FILES_LANGUAGES),
		("Loose\\ImageFormats", RELEASE_LOOSE_FILES_IMAGEFORMATS),
		("Loose\\Sharing", RELEASE_LOOSE_FILES_SHARING),
		("Loose", RELEASE_LOOSE_FILES_FACEBOOK),
		("Loose", RELEASE_LOOSE_FILES_WEB)
	]
	for dst in dstFolders:
		dstPath = os.path.join(buildDir, dst[0])
		if not releaseFilesIgnoreRegex.match(dst[0], re.IGNORECASE): #ignore if foldername matches
			LOG("Creating %s/%s directory" % (buildDir, dst[0]))
			if not os.path.exists(dstPath):
				os.mkdir(dstPath)
			for file in dst[1]:
				if not releaseFilesIgnoreRegex.match(file, re.IGNORECASE): #ignore if filename matches
					filePath = os.path.join(os.getcwd(), file)
					if os.path.exists(filePath):
						#LOG("  Copying %s to %s/%s" % (file, os.path.basename(buildDir), dst[0]))
						newFilePath = os.path.join(dstPath, os.path.basename(file))
						if os.path.isdir(filePath):
							shutil.copytree(filePath, newFilePath)
						else:
							shutil.copy2(filePath, newFilePath)
					else:
						LOG("  Error: Could not find %s" % filePath)
						sys.exit(1)
				else:
					LOG("  IGNORING: %s" % file)
		else:
			LOG("  IGNORING: %s" % dst)
	
def copySource(buildDir):
	global SOURCE_FILE_FILTER
	LOG("Creating %s/Source directory" % buildDir)	
	sourceDir = os.path.join(buildDir, "Source")
	os.mkdir(sourceDir)
	
	sourceFiles = os.listdir("Source")
	sourceFilesRegex = re.compile(SOURCE_FILE_FILTER)
	for file in sourceFiles:
		if sourceFilesRegex.match(file, re.IGNORECASE):		
			relFilePath = os.path.join("Source", file)
			filePath = os.path.join(os.getcwd(), relFilePath)
			# LOG("  Copying %s to %s/Source" % (relFilePath, os.path.basename(buildDir)))
			shutil.copy2(filePath, os.path.join(sourceDir, os.path.basename(file)))
	LOG("Copying Source files")
	
def grepFilesForPattern(patterns, fileMatch, failOnMatch, greaterThanZero, lessThanOne):
	#function not yet in use
	global GREP_BIN
	for pattern in patterns:
		cmd = ('%s %s ""%s""' % (GREP_BIN, pattern, fileMatch))
		grepProc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
		grepOut = grepProc.stdout.read()
		
		if (greaterThanZero):
			if (grepOut.find(pattern) > 0):
				LOG(grepOut)
				LOG("ERROR: Old CRTs found.  Cannot continue until old CRT dependencies are removed.")
				LOG("FOUND: " + pattern)
				if (failOnMatch):
					sys.exit(1)
		elif (lessThanOne):
			if (grepOut.find(pattern) < 1):
				if (failOnMatch):
					sys.exit(1)
				else:
					LOG("WARNING: NEW CRT NOT FOUND - UPDATE BUILD SCRIPT!")
	LOG("    Succeeded [%ds]" % (time.time() - start))

def testContainsBadCRT(objPattern, manifestDir):
	global oldCRTs, newCRTs, GREP_BIN, MT_BIN, MANIFEST_FILTER
	matches = False
	
	LOG("Ensuring no old CRT versions are being requested")
	start = time.time()
	LOG("  Scanning OBJs")
	for pattern in oldCRTs:
		cmd = ('%s %s ""%s""' % (GREP_BIN, pattern, objPattern))
		grepProc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
		grepOut = grepProc.stdout.read()
		if (grepOut.find(pattern) > 0):
			LOG(grepOut)
			LOG("ERROR: Old CRTs found.  Cannot continue until old CRT dependencies are removed.")
			LOG("FOUND: " + pattern)
			sys.exit(1)
	
	for pattern in newCRTs:
		cmd = ('%s %s ""%s""' % (GREP_BIN, pattern, objPattern))
		grepProc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
		grepOut = grepProc.stdout.read()
		if (grepOut.find(pattern) < 1):
			LOG("WARNING: NEW CRT NOT FOUND - UPDATE BUILD SCRIPT!")
	LOG("    Succeeded [%ds]" % (time.time() - start))
	
	LOG("  Extracting manifests from all compiled EXEs and DLLs")
	targetFiles = os.listdir(manifestDir)
	targetFilesRegex = re.compile(MANIFEST_FILTER)
	
	mtCmdFormat = "%s -inputresource:%s;#1 -out:%s"
	
	# generate .manifest files for all EXEs and DLLs
	#for file in targetFiles:
	#	if targetFilesRegex.match(file, re.IGNORECASE):
	#		relFilePath = os.path.join(manifestDir, file)
	#		filePath = os.path.join(os.getcwd(), relFilePath)
	#		mtCmd = mtCmdFormat % (MT_BIN, filePath, relFilePath + ".manifest")
	#		mtProc = subprocess.Popen(mtCmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	#		mtOut = mtProc.stdout.read()
	#
	# temporary hack to scan all .manifest file contents for bad CRT patterns
	# MANIFEST_FILTER = ".*.(exe|dll)$"
	
	#start = time.time()
	#LOG("  Scanning manifest files")
	#for pattern in oldCRTs:
	#	cmd = ('%s %s ""%s""' % (GREP_BIN, pattern, ".*.(exe|dll)$"))
	#	grepProc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	#	grepOut = grepProc.stdout.read()
	#	if (grepOut.find(pattern) > 0):
	#		LOG(grepOut)
	#		LOG("ERROR: Old CRTs found.  Cannot continue until old CRT dependencies are removed.")
	#		LOG("FOUND: " + pattern)
	#		sys.exit(1)
	#LOG("    Succeeded [%ds]" % (time.time() - start))
		
def signFiles(dir):
	global SIGN_FILE_FILTER
	LOG("Signing EXEs in directory %s" % dir)
	start = time.time()
	targetFiles = os.listdir(dir)
	targetFilesRegex = re.compile(SIGN_FILE_FILTER)

	for file in targetFiles:
		if targetFilesRegex.match(file, re.IGNORECASE):		
			relFilePath = os.path.join(dir, file)
			filePath = os.path.join(os.getcwd(), relFilePath)
			signFile(filePath)
	LOG("    Signing process took [%ds]" % (time.time() - start))
	
def signFile(file):
	global TIMESERVER, SIGNTOOL, signCertSHA1
	
	
	signCmd = '%s %s "%s"' % (SIGNTOOL, "sign /a /v /t " + TIMESERVER, file)
	signProc = subprocess.Popen(signCmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, bufsize=1)
	signOut = signProc.stdout.read()
	
	if (signOut.find("Number of warnings: 0") < 0) or (signOut.find("Number of errors: 0") < 0) or (signOut.find(signCertSHA1) < 0):
		LOG("ERROR: CODE SIGNING FAILED")
		LOG(signOut)
		LOG("    Ensure pfx is imported into machine store and correct SHA1 is specified in build script")
		LOG("    Skipping signing...")
		
# Delete files inside directories prior to its deletion """  
def deleteFiles(dirList, dirPath):   
    for file in dirList:   
		print "Deleting " + file   
		os.remove(dirPath + "/" + file)   

#Delete the directory itself 
def removeDirectory(dirEntry):   
    print "Deleting files in " + dirEntry[0]   
    deleteFiles(dirEntry[2], dirEntry[0])   
    emptyDirs.insert(0, dirEntry[0])  

def printUsage():	
	print "Usage: %s [-t (r|v|a)]" % sys.argv[0]
	print "  -t/--target (r[Release]|v[VIP]|a[All OEM and VGA])"
	print "  -v/--verbose  Output build results directly to console instead of build.log"
	print ""
	print "If no argument is specified, then you much select the release from the list"
	print "NB: You must have the Inno Installer installed in order to run this script."
	
if __name__ == "__main__":
	if not ensureDependencies():
		LOG("ERROR: Dependencies not met -- exiting.")
		sys.exit(1)

	print "Be sure everything in the solution is saved!! (Control + Shift + S)"
	
	# parse the command line args
	displayPrompt = True
	target = ""
	try:
		opts, args = getopt.getopt(sys.argv[1:], "vt:", ["target=","verbose"])
		for opt, arg in opts:
			if opt in ("-t", "--target"):
				displayPrompt = False
				target = arg
			elif opt in ("-v", "--verbose"):
				VERBOSE = True
	except getopt.GetoptError:
		printUsage()
		sys.exit(2)
		
	# display the prompt if necessary
	targetMap = {
		"r": ("Release", ["Release"]),
		"v": ("VIP", ["VIP"]),	
		"a": ("All OEM and VGA", ['OEM_JW', 'OEM_LENOVO', 'OEM_HP', 'OEM_ASUS', 'OEM_ACER', 'VGA_ASUS', 'VGA_TUL', 'VGA_COLORFUL', 'VGA_SAPPHIRE', 'VGA_HIS', 'VGA_INPAI']),
		"q": ("Exit this script", []),
		"vga_tul": ("VGA Tul", ['VGA_TUL']),
		"vga_sapphire": ("VGA Sapphire", ['VGA_SAPPHIRE']),
		"vga_colorful": ("VGA Colorful", ['VGA_COLORFUL']),
		"vga_his": ("VGA HIS", ['VGA_HIS']),
		"vga_inpai": ("VGA INPAI", ['VGA_INPAI']),
		"oem_jw": ("OEM_JW", ['OEM_JW']),
		"oem_lenovo": ("OEM_LENOVO", ['OEM_LENOVO']),
		"oem_hp": ("OEM_HP", ['OEM_HP']),
		"oem_intel_atom": ("OEM_INTEL_ATOM", ['OEM_INTEL_ATOM']),
		"tradeshow": ("TRADESHOW", ["TRADESHOW"]),
		"tradeshow_sapphire": ("TRADESHOW_SAPPHIRE", ["TRADESHOW_SAPPHIRE"])
	}
	while displayPrompt:
		print "\nPossible targets to build:"
		for t in targetMap:
			print '"%s"\t\t%s' % (t, targetMap[t][0])
		target = raw_input("Enter target to build: ")
		
		if target in targetMap:
			if target == "q":
				print "Build cancelled"
				sys.exit(0)	
			break;
		else:
			print "ERROR: Unknown target"		
			
	start = time.time()
			
	# convert target to versions
	if not target in targetMap:
		print "Invalid target specified: %s" % target
		printUsage()
		sys.exit(2)
	print "Building target: %s" % targetMap[target][0]
	VERSIONS = targetMap[target][1]
	
	alterResourceVersion(svnRev)
	cleanProjectOutputs()
	buildProjectOutputs(VERSIONS)
	testContainsBadCRT("Source\\Release\\tmp\\*.obj", "Source\\Release\\")
	testContainsBadCRT("Source\\Release_GL\\tmp\\*.obj", "Source\\Release_GL\\")
	#runTests()
	
	for version in VERSIONS:
		# create a build directory
		buildDir = createBuildDirectory(version, svnRev)
		LOG(buildDir)
		# alter the exe
		alterReleaseVersion(buildDir, version)
		# build the installer and copy over the loose files
		signFiles(os.path.join(os.getcwd(), "Source\\Release\\"))
		signFiles(os.path.join(os.getcwd(), "Source\\Release\\x64\\"))
		preprocessFiles()
		buildBumpTopInstaller(buildDir, version)
		postprocessFiles()
		signFiles(buildDir)
		generateMD5s(buildDir)
		copyLoose(buildDir)
		buildSevenZip(buildDir) #for stub
		# copy over the Source dir for debugging purposes
		copySource(buildDir)
		# build loose and source - DO NOT remove pdbs from this archive
		compressSevenZip(os.path.join(buildDir, ""), ' a -xr!.svn -xr!tmp -xr!*.msi -xr!*.psd -mx=9 -ms -mmt=on "%s" "Source\\*" "Loose\\*" -mx=9 -ms -mmt=on', buildDir + "\\internalLooseNSource" + str(svnRev) + ".7z")
		#remove Source and Loose folders
		LOG("Removing Loose and Source folders")
		os.system(('attrib -h -s -r /s "%s\\*"' % (buildDir + "\\Loose")))
		shutil.rmtree(buildDir + "\\Source", True)
		shutil.rmtree(buildDir + "\\Loose", True)
	# XXX: move the build log over
	
	# rename the log to suceeded
	
	diffTime =  (time.time() - start)
	LOG("-------------------- All done! --------------------")
	LOG(', '.join(VERSIONS))
	LOG("  Succeeded [%dm %ds]" % (diffTime / 60, diffTime % 60))
	LOG("---------------------------------------------------")
	
	# close the log
	END_LOG()
