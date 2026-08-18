# BumpTop Modern macOS (Apple Silicon) Port — Status

Branch: `port/modern-macos`. The app **builds and runs natively on Darwin arm64**
(developed against macOS 26 / Xcode 26). It renders the 3D room at desktop
level behind your windows, imports the real desktop (files, volumes, Finder
icon positions), loads QuickLook thumbnails, runs physics, and behaves like
the desktop across Spaces/Mission Control.

## Building

```bash
brew install cmake ninja pkg-config qt boost protobuf bullet

# One-time: build the patched static Ogre 14 into .port-deps/
git clone --depth 1 --branch v14.4.1 https://github.com/OGRECave/ogre.git .port-deps/ogre-src
# (apply the render-order patch — see below — then:)
cmake -S .port-deps/ogre-src -B .port-deps/ogre-build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DCMAKE_INSTALL_PREFIX=$PWD/.port-deps/ogre-install \
  -DOGRE_STATIC=TRUE -DOGRE_BUILD_LIBS_AS_FRAMEWORKS=FALSE \
  -DOGRE_BUILD_RENDERSYSTEM_GL=TRUE -DOGRE_BUILD_RENDERSYSTEM_GL3PLUS=FALSE \
  -DOGRE_BUILD_RENDERSYSTEM_METAL=FALSE -DOGRE_BUILD_RENDERSYSTEM_GLES2=FALSE \
  -DOGRE_BUILD_RENDERSYSTEM_VULKAN=FALSE -DOGRE_BUILD_RENDERSYSTEM_TINY=FALSE \
  -DOGRE_BUILD_COMPONENT_OVERLAY=TRUE -DOGRE_BUILD_COMPONENT_OVERLAY_IMGUI=FALSE \
  -DOGRE_BUILD_COMPONENT_BITES=FALSE -DOGRE_BUILD_COMPONENT_BULLET=FALSE \
  -DOGRE_BUILD_COMPONENT_TERRAIN=FALSE -DOGRE_BUILD_COMPONENT_VOLUME=FALSE \
  -DOGRE_BUILD_COMPONENT_PAGING=FALSE -DOGRE_BUILD_COMPONENT_MESHLODGENERATOR=FALSE \
  -DOGRE_BUILD_COMPONENT_PROPERTY=FALSE -DOGRE_BUILD_COMPONENT_RTSHADERSYSTEM=FALSE \
  -DOGRE_BUILD_PLUGIN_DOT_SCENE=FALSE -DOGRE_BUILD_PLUGIN_ASSIMP=FALSE \
  -DOGRE_BUILD_PLUGIN_BSP=FALSE -DOGRE_BUILD_PLUGIN_OCTREE=FALSE \
  -DOGRE_BUILD_PLUGIN_PCZ=FALSE -DOGRE_BUILD_PLUGIN_PFX=FALSE \
  -DOGRE_BUILD_PLUGIN_STBI=TRUE -DOGRE_NODELESS_POSITIONING=TRUE \
  -DOGRE_RESOURCEMANAGER_STRICT=0 \
  -DOGRE_BUILD_SAMPLES=FALSE -DOGRE_BUILD_TOOLS=FALSE -DOGRE_INSTALL_DOCS=FALSE
cmake --build .port-deps/ogre-build && cmake --install .port-deps/ogre-build

# The app itself
cmake -S trunk/mac -B build -G Ninja
cmake --build build
build/BumpTop.app/Contents/MacOS/BumpTop   # or open build/BumpTop.app
```

Quit via `pkill -x BumpTop` or the menu-bar item.

### Ogre flags that matter
- `OGRE_NODELESS_POSITIONING=TRUE` — restores Camera/Light setPosition/lookAt
  (the codebase uses Ogre 1.7-style nodeless transforms everywhere).
- `OGRE_RESOURCEMANAGER_STRICT=0` — legacy resource lookup; the app loads
  textures by absolute path.
- Static GL render system (legacy OpenGL runs fine on Apple Silicon).
- **Vendored patch** (`#ManuallySpecifyingRenderOrder`) in
  `OgreMain/include/OgreRenderable.h` + `OgreMain/src/OgreRenderQueueSortingGrouping.cpp`:
  re-adds BumpTop's setToRenderBefore/After render-order ties used for
  labels/highlights over icons (grep the marker in `.port-deps/ogre-src`).

## What was ported (see git log for detail)
- Legacy Xcode 3 project → CMake/Ninja (`trunk/mac/CMakeLists.txt`); all
  sources compile as Objective-C++ with `BumpTop/port_prefix.h` force-included.
- Qt 4.6 → Qt 6.11 (Widgets split, Core5Compat QRegExp, QHttp →
  QNetworkAccessManager, qSort shim, AUTOMOC instead of checked-in moc files).
- Ogre 1.7 → 14.4 (overlay system registration, static plugin install in
  `BumpTopApp::createRootNode`, SharedPtr/AxisAlignedBox/Overlay API updates,
  `Resource::Listener::loadingComplete` rename — this one mattered: textures
  never applied without it).
- Bullet 2.78 → brew Bullet 3 (API compatible), protobuf 2.4 → 35
  (regenerated `AllMessages.pb.*`; generated classes un-`final`ed because the
  app inherits from them — see `trunk/mac/BumpTop/protoc/`).
- Carbon removal: Process Manager → NSRunningApplication, theme cursors →
  NSCursor, Carbon Menu Manager context menu → NSMenu
  (`OSX/ContextMenu.cpp`), NSStatusItem private ivar → button API.
- Sparkle/CMCrashReporter/LetsMove stubbed (`trunk/mac/PortShims/`);
  QtWebKit theme browser and pre-10.6 QuickLook excluded.
- Retina: GL surface at native resolution, mouse coords in device pixels,
  room/floor in points, labels and background textures rendered at 2x.
- Desktop behavior: borderless window covering the whole screen at
  `kCGDesktopIconWindowLevel` with CanJoinAllSpaces/Stationary/IgnoresCycle —
  correct Exposé / Mission Control / Spaces semantics on modern macOS.
- QuickLook icon loading normalized to 8-bit RGBA and falls back to the
  standard file icon when the deprecated `QLThumbnailImageCreate` fails.

## Desktop parity test (`trunk/mac/parity/`)
`BUMPTOP_PARITY=1` renders the room like the flat desktop: birds-eye camera,
uniform lighting, the actual wallpaper as the floor (no vignette), no sticky
pad / new-items pile / tooltip, Finder's icon size, and periodic framebuffer
dumps to `~/Library/Application Support/BumpTop/parity_render.png`.

```bash
zsh trunk/mac/parity/parity_run.sh /tmp/bt-parity
# knobs: BUMPTOP_PARITY_OFFSET_X/_Y (position calibration, points)
#        BUMPTOP_PARITY_SELECT=<desktop file name>  (selection-rendering test)
```
It captures the real desktop (wallpaper + Finder icon windows; cached copies
in App Support are reused when a Space/full-screen app blocks window capture),
composites a reference, and pixel-diffs (`diff_images.py` → stats, heatmap,
side-by-side). Progression so far: 98% → 3.6% of pixels differing >16/255
(wallpaper is pixel-identical; residual is icon/label position calibration).
Don't touch the mouse/desktop during a run — it re-imports positions.

## Known gaps / next steps
- Parity: finish position/label-offset calibration; label typography
  (Finder uses SF, BumpTop Lucida Grande bold 13); selection-rendering diff.
- A second BumpTop window shows in Mission Control previews occasionally
  (the 0x0 helper window) — cosmetic.
- Sticky notes render but font/colors need checking; Pro-license gating still
  in place (`ProAuthorization`), server long dead.
- Legacy deprecations still in use deliberately: OpenGL, NSOpenGLContext,
  QLThumbnailImageCreate, LSSharedFileList (login items), AppleScript Finder
  automation (may prompt for permission on first run).
- `.port-deps/` is gitignored; document or vendor the Ogre patch if the
  machine changes.
