#  Copyright 2012 Google Inc. All Rights Reserved.
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

all: bullet boost-thread ogre-built qt protobuf sparkle CMCrashReporterModule gtest

bullet: bullet-2.78/src/BulletDynamics/MinSizeRel/libBulletDynamics.a bullet-2.78/src/BulletCollision/MinSizeRel/libBulletCollision.a bullet-2.78/src/LinearMath/MinSizeRel/libLinearMath.a

ogre-built: ogre/lib/MinSizeRel/Ogre.framework boost-thread

ogre_src_v1-7-3/._.hgtags: ogre_src_v1-7-3.tar.gz
	rm -rf temp
	mkdir temp
	cd temp; gunzip --stdout ../ogre_src_v1-7-3.tar.gz | tar -xkv
	# copy without overwriting existing files
	rsync -vr --ignore-existing temp/ogre_src_v1-7-3 .
	rm -r temp
	touch ogre_src_v1-7-3/._.hgtags

ogre/Dependencies:
	rm -rf ogre/Dependencies
	unzip -o OgreDependencies_OSX_20110208.zip
	mkdir -p ogre
	mv Dependencies ogre
	rm -rf __MACOSX

ogre/lib/MinSizeRel/Ogre.framework: ogre_src_v1-7-3/._.hgtags ogre/Dependencies
	cd ogre; BOOST_ROOT=../boost/ cmake ../ogre_src_v1-7-3 -GXcode; cd ..
	xcodebuild -project ogre/OGRE.xcodeproj -parallelizeTargets -configuration MinSizeRel -target OgreMain -target RenderSystem_GL ARCHS="i386"

bullet-2.78/src/BulletDynamics/MinSizeRel/libBulletDynamics.a: bullet-2.78/BULLET_PHYSICS.xcodeproj
	xcodebuild -target BulletDynamics -project $< -configuration MinSizeRel -parallelizeTargets ARCHS="i386" GCC_SYMBOLS_PRIVATE_EXTERN="YES"
	touch $@

bullet-2.78/src/BulletCollision/MinSizeRel/libBulletCollision.a: bullet-2.78/BULLET_PHYSICS.xcodeproj
	xcodebuild -target BulletCollision -project $< -configuration MinSizeRel -parallelizeTargets ARCHS="i386" GCC_SYMBOLS_PRIVATE_EXTERN="YES"
	touch $@

bullet-2.78/src/LinearMath/MinSizeRel/libLinearMath.a: bullet-2.78/BULLET_PHYSICS.xcodeproj
	xcodebuild -target LinearMath -project $< -configuration MinSizeRel -parallelizeTargets ARCHS="i386" GCC_SYMBOLS_PRIVATE_EXTERN="YES"
	touch $@

CMCrashReporter: CMCrashReporter/build/Release/CMCrashReporter.framework

CMCrashReporter/build/Release/CMCrashReporter.framework: CMCrashReporter/CMCrashReporter.xcodeproj CMCrashReporter/English.lproj/CMCrashReporterWindow.nib
	xcodebuild -target CMCrashReporter -project CMCrashReporter/CMCrashReporter.xcodeproj -configuration Release -parallelizeTargets ARCHS="i386" SDKROOT=""

CMCrashReporterModule: CMCrashReporterModule/build/Release/CMCrashReporter.app

CMCrashReporterModule/build/Release/CMCrashReporter.app: CMCrashReporterModule/CMCrashReporterModule.xcodeproj CMCrashReporter
	xcodebuild -target CMCrashReporterModule -project $< -configuration Release -parallelizeTargets ARCHS="i386" SDKROOT=""

bullet-2.78/BULLET_PHYSICS.xcodeproj: bullet-2.78
	cd $<; cmake . -G Xcode; cd ..

bullet-2.78: bullet-2.78.zip
	cmake --version
	# (just to force user to notice immediately if cmake isn't installed)
	unzip -o $<
	touch $@

sparkle: andymatuschak-Sparkle-af37928/build/Release/Sparkle.framework

andymatuschak-Sparkle-af37928/build/Release/Sparkle.framework: andymatuschak-Sparkle-af37928/Sparkle.xcodeproj
	xcodebuild -target Sparkle -project $< -configuration Release -parallelizeTargets ARCHS="i386" SDKROOT=""
	touch $@

qt: qtcore qtnetwork qtgui qtlockedfile moc qtxmlpatterns qtwebkit qtimageformats

qtcore: QtCore.framework

QtCore.framework: QtCore-Cocoa_32-bit-4.6.1.framework.zip
	unzip -o $<
	install_name_tool -id "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" QtCore.framework/Versions/4/QtCore
	touch $@

qtgui: QtGui.framework

QtGui.framework: QtGui-Cocoa_32-bit-4.6.1.framework.zip
	unzip -o $<
	install_name_tool -id "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" QtGui.framework/Versions/4/QtGui
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtGui.framework/QtGui
	touch $@

qtnetwork: QtNetwork.framework

QtNetwork.framework: QtNetwork-Cocoa_32-bit-4.6.1.framework.zip
	unzip -o $<
	install_name_tool -id "@executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork" QtNetwork.framework/Versions/4/QtNetwork
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtNetwork.framework/QtNetwork
	touch $@

qtxmlpatterns: QtXmlPatterns.framework

QtXmlPatterns.framework: QtXmlPatterns-Cocoa_32-bit-4.6.1.framework.zip
	unzip -o $<
	install_name_tool -id "@executable_path/../Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns" QtXmlPatterns.framework/Versions/4/QtXmlPatterns
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtXmlPatterns.framework/QtXmlPatterns
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork QtXmlPatterns.framework/QtXmlPatterns
	touch $@

qtwebkit: QtWebKit.framework

QtWebKit.framework: QtWebKit-Cocoa_32-bit-4.6.1.framework.zip
	unzip -o $<
	install_name_tool -id "@executable_path/../Frameworks/QtWebKit.framework/Versions/4/QtWebKit" QtWebKit.framework/Versions/4/QtWebKit
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtXmlPatterns.framework/Versions/4/QtXmlPatterns @executable_path/../Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns QtWebKit.framework/QtWebKit
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui QtWebKit.framework/QtWebKit
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtNetwork.framework/Versions/4/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork QtWebKit.framework/QtWebKit
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore QtWebKit.framework/QtWebKit
	touch $@

qtimageformats: qjpeg qgif qmng
	
qjpeg: qtimageformats/libqjpeg.dylib
	install_name_tool -id "@executable_path/imageformats/libqjpeg.dylib" $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $<

qgif: qtimageformats/libqgif.dylib
	install_name_tool -id "@executable_path/imageformats/libqgif.dylib" $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $<	

qmng: qtimageformats/libqmng.dylib	
	install_name_tool -id "@executable_path/imageformats/libqmng.dylib" $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui $<
	install_name_tool -change /usr/local/Trolltech/Qt-4.6.1/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore $<

moc: moc-Cocoa_32-bit-4.6.1.zip
	unzip -o $<
	touch $@

qtlockedfile: qtlockedfile-2.4-opensource

qtlockedfile-2.4-opensource: qtlockedfile-2.4-opensource.tar.gz
	gunzip --stdout $< | tar -xv
	install_name_tool -id "@executable_path/../SharedSupport/libQtSolutions_LockedFile-2.4.1.0.0.dylib" qtlockedfile-2.4-opensource/lib/libQtSolutions_LockedFile-2.4.1.0.0.dylib
	install_name_tool -change /usr/local/Trolltech/Qt-4.5.2/lib/QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui qtlockedfile-2.4-opensource/lib/libQtSolutions_LockedFile-2.4.1.0.0.dylib
	install_name_tool -change /usr/local/Trolltech/Qt-4.5.2/lib/QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore qtlockedfile-2.4-opensource/lib/libQtSolutions_LockedFile-2.4.1.0.0.dylib
	touch $@

ogre-prebuilt: ogre/Mac/build/Release/Ogre.framework/Ogre

ogre/Mac/build/Release/Ogre.framework/Ogre: ogre/Mac/build/Release/Ogre.framework.zip ogre/Mac/SamplesE
	cd ogre/Mac/build/Release; unzip -o Ogre.framework.zip; touch Ogre.framework/Ogre

#ogre-built: ogre/Mac/build/Release/Ogre.framework

ogre/Mac/build/Release/Ogre.framework: boost-thread ogre/Mac/SamplesE ogre/Dependencies
	xcodebuild -project ogre/Mac/Ogre/Ogre.xcodeproj -parallelizeTargets
	touch $@

#ogre/Dependencies: ogre OgreDependencies_OSX_Eihort_20080115
#	cp -r OgreDependencies_OSX_Eihort_20080115/Dependencies ogre
#	touch $@

OgreDependencies_OSX_Eihort_20080115: OgreDependencies_OSX_Eihort_20080115.zip
	unzip -o $<
	touch $@
	rm -rf __MACOSX

ogre/Mac/SamplesE: ogre-v1-6-4.zip
	unzip -u $<
	touch $@
	#chmod -R u+w ogre/Docs/html

boost_1_48_0/b2: boost_1_48_0
	cd boost_1_48_0; ./bootstrap.sh --prefix=../boost
	touch $@

boost-thread: boost/lib/libboost_thread.dylib

boost/lib/libboost_thread.dylib: boost_1_48_0/b2 boost_1_48_0
	cd boost_1_48_0; ./b2 cflags="-fvisibility=hidden -fvisibility-inlines-hidden" cxxflags="-fvisibility=hidden -fvisibility-inlines-hidden" architecture=x86 address-model=32 -d+2 --toolset=darwin-4.2 --build-type=minimal --with-thread --with-date_time install; cd ..
	install_name_tool -id "@executable_path/../SharedSupport/libboost_thread.dylib" boost/lib/libboost_thread.dylib
	install_name_tool -id "@executable_path/../SharedSupport/libboost_date_time.dylib" boost/lib/libboost_date_time.dylib
	touch boost/lib/libboost_thread.dylib
	touch boost/lib/libboost_date_time.dylib

boost_1_48_0: boost_1_48_0.tar.gz
	gunzip --stdout $< | tar -xv
	touch $@

gtest: gtest-1.3.0/xcode/build/Release/gtest.framework

gtest-1.3.0/xcode/build/Release/gtest.framework: gtest-1.3.0
	xcodebuild -project gtest-1.3.0/xcode/gtest.xcodeproj -parallelizeTargets ARCHS="i386" SDKROOT="" GCC_VERSION=""

gtest-1.3.0: gtest-1.3.0.tar.gz
	gunzip --stdout $< | tar -xv
	touch $@
	chmod -R +x $@
	patch gtest-1.3.0/include/gtest/internal/gtest-port.h < gtest-tr1-fix.patch

protobuf: protobuf-2.4.1/build/lib/libprotobuf.a protobuf-2.4.1/build/lib/dylib

protobuf-2.4.1/build/lib/dylib: protobuf-2.4.1/build/lib/libprotobuf.a
	[ -d "$@" ] || mkdir "$@"
	mv protobuf-2.4.1/build/lib/*.dylib "$@"
	touch "$@"

protobuf-2.4.1/build/lib/libprotobuf.a: protobuf-2.4.1
	cd protobuf-2.4.1; /usr/bin/env CFLAGS='-arch i386 -arch x86_64' LDFLAGS='-arch i386 -arch x86_64' CXXFLAGS='-arch i386 -arch x86_64' sh ./configure --prefix=$$PWD/build --disable-dependency-tracking; make; make install; cd ..

protobuf-2.4.1: protobuf-2.4.1.tar.gz
	gunzip --stdout $< | tar -xv
	touch $@
