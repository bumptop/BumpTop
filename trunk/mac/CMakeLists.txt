# Modern macOS (arm64) port build for BumpTop.
# Replaces the legacy Xcode 3 project in Build/Mac/BumpTop.xcodeproj.
#
# Prerequisites:
#   brew install cmake ninja qt boost protobuf bullet
#   Ogre 14.x built statically into ../../.port-deps/ogre-install (see HANDOFF.md)
#
# Build:
#   cmake -S trunk/mac -B build -G Ninja && cmake --build build

cmake_minimum_required(VERSION 3.21)
project(BumpTop LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_OSX_DEPLOYMENT_TARGET "26.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(MAC_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(REPO_ROOT ${MAC_ROOT}/../.. ABSOLUTE)
set(PORT_DEPS ${REPO_ROOT}/.port-deps)

list(APPEND CMAKE_PREFIX_PATH /opt/homebrew ${PORT_DEPS}/ogre-install)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Core5Compat)
find_package(OGRE CONFIG REQUIRED PATHS ${PORT_DEPS}/ogre-install/CMake)
find_package(Protobuf CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(Bullet CONFIG REQUIRED PATHS /opt/homebrew/lib/cmake/bullet NO_DEFAULT_PATH)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
file(GLOB BT_CORE_SOURCES ${MAC_ROOT}/BumpTop/*.cpp)
file(GLOB BT_UNDO_SOURCES ${MAC_ROOT}/BumpTop/UndoCommands/*.cpp)
file(GLOB BT_OSX_MM_SOURCES ${MAC_ROOT}/BumpTop/OSX/*.mm)
file(GLOB BT_OSX_CPP_SOURCES ${MAC_ROOT}/BumpTop/OSX/*.cpp)

set(BT_ALL_SOURCES
    ${BT_CORE_SOURCES} ${BT_UNDO_SOURCES}
    ${BT_OSX_MM_SOURCES} ${BT_OSX_CPP_SOURCES})

# Tests and standalone tools are not part of the app target.
list(FILTER BT_ALL_SOURCES EXCLUDE REGEX "Test(Helper)?s?\\.(cpp|mm)$")
list(FILTER BT_ALL_SOURCES EXCLUDE REGEX "CreateConfigFile\\.cpp$")
# Pre-10.6 QuickLook private-API path; the app selects the 10.6+ path at runtime.
list(FILTER BT_ALL_SOURCES EXCLUDE REGEX "QuickLookLeopard\\.mm$")
# Embedded-QtWebKit theme browser for a defunct website; its one call site is
# commented out upstream.
list(FILTER BT_ALL_SOURCES EXCLUDE REGEX "ThemeDownloader\\.cpp$")

set(THIRD_PARTY_SOURCES
    ${MAC_ROOT}/PortShims/SparkleStub.mm
    ${MAC_ROOT}/PortShims/PFMoveApplicationStub.mm
    ${MAC_ROOT}/Dependencies/ThirdParty/CppTweener.cpp
    ${MAC_ROOT}/Dependencies/ThirdParty/BlitzBlur.cpp
    ${MAC_ROOT}/Dependencies/ThirdParty/GetPrimaryMACAddress.cpp
    ${MAC_ROOT}/Dependencies/qtlockedfile-2.4-opensource/src/qtlockedfile.cpp
    ${MAC_ROOT}/Dependencies/qtlockedfile-2.4-opensource/src/qtlockedfile_unix.cpp
)

set(PROTO_SOURCES ${MAC_ROOT}/BumpTop/protoc/AllMessages.pb.cc)

add_executable(BumpTop MACOSX_BUNDLE
    ${BT_ALL_SOURCES}
    ${THIRD_PARTY_SOURCES}
    ${PROTO_SOURCES}
)

# ---------------------------------------------------------------------------
# Compile setup
# ---------------------------------------------------------------------------
# NOTE: ${MAC_ROOT}/BumpTop must NOT be an include dir — on a case-insensitive
# filesystem its Math.h would shadow the system <math.h>.
target_include_directories(BumpTop PRIVATE
    ${MAC_ROOT}                       # "BumpTop/..." includes
    ${MAC_ROOT}/PortShims             # stub <Sparkle/Sparkle.h>
    ${MAC_ROOT}/Dependencies          # "ThirdParty/..." includes
    ${MAC_ROOT}/Dependencies/ThirdParty
    ${MAC_ROOT}/Dependencies/qtlockedfile-2.4-opensource/src
    ${PORT_DEPS}/ogre-src/RenderSystems/GLSupport/include  # OSX/OgreOSXCocoaView.h
    ${PORT_DEPS}/ogre-install/include/OGRE/RenderSystems/glad
    ${PORT_DEPS}/ogre-install/include/OGRE/RenderSystems/KHR
    /opt/homebrew/include/bullet
)

# Every TU picks up the legacy prefix header (Qt/Ogre/Bullet/boost umbrella).
# The whole codebase is Objective-C++ (the old Xcode project compiled .cpp files
# as ObjC++ too; they use Cocoa types directly).
target_compile_options(BumpTop PRIVATE
    -x objective-c++
    -include ${MAC_ROOT}/BumpTop/port_prefix.h
    -Wno-deprecated-declarations
    -Wno-inconsistent-missing-override
    -Wno-writable-strings
)

target_compile_definitions(BumpTop PRIVATE OGRE_STATIC_LIB OS_MACOSX)

link_directories(${BULLET_LIBRARY_DIRS})

target_link_directories(BumpTop PRIVATE /opt/homebrew/lib)

target_link_libraries(BumpTop PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Core5Compat
    OgreMain OgreOverlay OgreGLSupport RenderSystem_GL Codec_STBI
    protobuf::libprotobuf
    BulletDynamics BulletCollision LinearMath
    "-framework Cocoa"
    "-framework AppKit"
    "-framework Foundation"
    "-framework OpenGL"
    "-framework IOKit"
    "-framework CoreServices"
    "-framework ApplicationServices"
    "-framework Quartz"
    "-framework QuartzCore"
    "-framework CoreText"
    "-framework Security"
)

# ---------------------------------------------------------------------------
# Bundle: Info.plist, nibs, resources
# ---------------------------------------------------------------------------
set_target_properties(BumpTop PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${MAC_ROOT}/Build/Mac/Info-port.plist
    OUTPUT_NAME "BumpTop"
)

set(APP_RESOURCES_DIR $<TARGET_BUNDLE_CONTENT_DIR:BumpTop>/Resources)

set(BT_XIBS
    ${MAC_ROOT}/Build/Mac/English.lproj/MainMenu.xib
    ${MAC_ROOT}/Build/Mac/English.lproj/PreferencesWindow.xib
    ${MAC_ROOT}/Build/Mac/English.lproj/GeneralSettingsWindow.xib
    ${MAC_ROOT}/Build/Mac/English.lproj/TextureSettingsBox.xib
    ${MAC_ROOT}/Build/Mac/English.lproj/CompressProgressDialog.xib
    # ContextMenu.xib is a Carbon-format xib; the port builds that menu in
    # code with NSMenu (see OSX/ContextMenu.cpp).
    ${MAC_ROOT}/Build/Mac/ProSettingsWindow.xib
)

foreach(XIB ${BT_XIBS})
    get_filename_component(XIB_NAME ${XIB} NAME_WE)
    add_custom_command(TARGET BumpTop POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${APP_RESOURCES_DIR}/English.lproj"
        COMMAND ibtool --compile "${APP_RESOURCES_DIR}/English.lproj/${XIB_NAME}.nib" "${XIB}"
        COMMENT "Compiling ${XIB_NAME}.xib"
    )
endforeach()

add_custom_command(TARGET BumpTop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MAC_ROOT}/Graphics "${APP_RESOURCES_DIR}"
    # The resource location is indexed non-recursively, so flatten LightBox
    # (floor_overlay.png / wall_overlay.png) into Resources.
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MAC_ROOT}/Graphics/LightBox "${APP_RESOURCES_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy ${MAC_ROOT}/Build/Mac/BUMPDEV.icns "${APP_RESOURCES_DIR}/"
    COMMENT "Copying resources"
)
