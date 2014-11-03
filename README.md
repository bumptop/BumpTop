bumptop
=======

Although BumpTop was acquired by Google, this is not an official Google product.  We are excited to have folks develop on top of our work and it is presented here under the Apache license but unfortunately the BumpTop team does not have time to maintain it and so contributions to the main product will not be accepted, but fork away!  

Mac Build instructions
======================
Prerequisites: Xcode, Mac OS X 10.6+

Checkout trunk/mac from the source tree.  
Download and install [CMake](http://www.cmake.org/cmake/resources/software.html)  
Open Terminal.app, go to the trunk/mac/Dependencies directory, and type "make" (this will take over 20 minutes)  
Launch the BumpTop Xcode project (trunk/mac/Build/Mac/BumpTop.xcodeproj)  
Select the "BumpTop" target, or if you'd like to build a downloadable .dmg, select the "Deploy" target  
Build the project


Windows Build instructions
==========================
Prerequisites: [Microsoft Visual C++ 2010 Express](http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express) (free), [Python 2.6](http://python.org/getit/) installed and added to your path.

Checkout trunk/win from the source tree.  
Install DirectX 9 SDK, June 2010 release [download](http://www.microsoft.com/en-us/download/details.aspx?id=6812)  
Download the [Qt 4.6.1 source](http://download.qt-project.org/archive/qt/4.6/qt-everywhere-opensource-src-4.6.1.tar.gz) into your /Dependencies folder  
Open BumpTop.sln in Visual Studio, right-click on BumpTop project and select "Set as StartUp Project"  


Flash Build instructions
========================
Prerequisites: [Flex 4.6 SDK](http://www.adobe.com/devnet/flex/flex-sdk-download.html). [Flash Builder 4.6](http://www.adobe.com/cfusion/tdrc/index.cfm?product=flash_builder) to follow the below instructions. Note: it was originally built against Flex 3 so you may see some warnings.

Checkout trunk/flash from the source tree.  
File -> New -> Flex Project named "TileUI"  
Step through wizard  
Copy the files under src to src  
Copy the files under libs to libs  
Delete TileUI.mxml (which was created automatically by Flash Builder when setting up the new project)  
Right click TileUI_Flex.mxml -> Debug as… -> Web Application  
(Note: you may need to install the [debugger version of Flash  Player](http://www.adobe.com/support/flashplayer/downloads.html) and for Google Chrome you need to [disable the built-in version of Flash](http://www.aaronwest.net/blog/index.cfm/2010/4/27/Configuring-Chrome-with-Flash-Player-Debugger))
