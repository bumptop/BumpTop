QT_LIB_DIR=../../Dependencies/Qt-Win_x86-4.5.2-VC9/lib_x86/
OGRE_LIB_DIR=../../Dependencies/ogre/lib/
CP=..\..\Dependencies\Windows\cp
GRAPHICS_DIR=../../Graphics/

all: Debug/QtCore4.dll Debug/QtGui4.dll Debug/OgreMain_d.dll Release/QtCore4.dll Release/QtGui4.dll Release/OgreMain.dll Debug/resources.cfg Release/resources.cfg Release/RenderSystem_Direct3D9.dll Debug/RenderSystem_Direct3D9_d.dll Debug/CppTweener.dll Release/CppTweener.dll Release/floor_desktop.jpg Release/square_texture.png Release/wall.png Release/wall_front.jpg Debug/floor_desktop.jpg Debug/square_texture.png Debug/wall.png Debug/wall_front.jpg


Debug/CppTweener.dll: ../../Dependencies/cpptweener/Release/CppTweener.dll
	$(CP) ../../Dependencies/cpptweener/Release/CppTweener.dll Debug

Release/CppTweener.dll: ../../Dependencies/cpptweener/Release/CppTweener.dll
	$(CP) ../../Dependencies/cpptweener/Release/CppTweener.dll Release

Debug/QtCore4.dll: $(QT_LIB_DIR)QtCore4.dll
	$(CP) $(QT_LIB_DIR)QtCore4.dll Debug

Debug/QtGui4.dll: $(QT_LIB_DIR)QtGui4.dll
	$(CP) $(QT_LIB_DIR)QtGui4.dll Debug

Debug/OgreMain_d.dll: $(OGRE_LIB_DIR)OgreMain_d.dll
	$(CP) $(OGRE_LIB_DIR)OgreMain_d.dll Debug

Release/QtCore4.dll: $(QT_LIB_DIR)QtCore4.dll
	$(CP) $(QT_LIB_DIR)QtCore4.dll Release

Release/QtGui4.dll: $(QT_LIB_DIR)QtGui4.dll
	$(CP) $(QT_LIB_DIR)QtGui4.dll Release

Release/OgreMain.dll: $(OGRE_LIB_DIR)OgreMain.dll
	$(CP) $(OGRE_LIB_DIR)OgreMain.dll Release

Debug/resources.cfg: ../../BumpTop/resources.cfg
	$(CP) ../../BumpTop/resources.cfg Debug

Release/resources.cfg: ../../BumpTop/resources.cfg
	$(CP) ../../BumpTop/resources.cfg Release

Debug/RenderSystem_Direct3D9_d.dll: $(OGRE_LIB_DIR)RenderSystem_Direct3D9_d.dll
	$(CP) $(OGRE_LIB_DIR)RenderSystem_Direct3D9_d.dll Debug

Release/RenderSystem_Direct3D9.dll: $(OGRE_LIB_DIR)RenderSystem_Direct3D9.dll
	$(CP) $(OGRE_LIB_DIR)RenderSystem_Direct3D9.dll Release

Release/floor_desktop.jpg: $(GRAPHICS_DIR)floor_desktop.jpg
	$(CP) $(GRAPHICS_DIR)floor_desktop.jpg Release

Release/square_texture.png: $(GRAPHICS_DIR)square_texture.png
	$(CP) $(GRAPHICS_DIR)square_texture.png Release

Release/wall.png: $(GRAPHICS_DIR)wall.png
	$(CP) $(GRAPHICS_DIR)wall.png Release

Release/wall_front.jpg: $(GRAPHICS_DIR)wall_front.jpg
	$(CP) $(GRAPHICS_DIR)wall_front.jpg Release

Debug/floor_desktop.jpg: $(GRAPHICS_DIR)floor_desktop.jpg
	$(CP) $(GRAPHICS_DIR)floor_desktop.jpg Debug

Debug/square_texture.png: $(GRAPHICS_DIR)square_texture.png
	$(CP) $(GRAPHICS_DIR)square_texture.png Debug

Debug/wall.png: $(GRAPHICS_DIR)wall.png
	$(CP) $(GRAPHICS_DIR)wall.png Debug

Debug/wall_front.jpg: $(GRAPHICS_DIR)wall_front.jpg
	$(CP) $(GRAPHICS_DIR)wall_front.jpg Debug