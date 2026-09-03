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
git -C .port-deps/ogre-src apply "$PWD/trunk/mac/ogre-14-render-order.patch"
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

### Packaging a DMG
`./trunk/mac/package_dmg.sh` → `dist/BumpTop.dmg`: builds, bundles Qt via
macdeployqt, signs, and wraps in a branded DMG (background art, Applications
alias; the Finder-layout scripting needs Automation permission and is skipped
under CI).

## CI, signing, and releases

`.github/workflows/build-macos.yml` builds and packages the app on an Apple
Silicon runner: manually (Actions → *Build macOS* → *Run workflow*), on every
push to `master`, and on PRs targeting `master`. The patched Ogre is built
once and cached (keyed on the Ogre version + the vendored patch); each run
uploads `dist/BumpTop.dmg` as an artifact (90-day retention) after
verification. Nothing is published anywhere — releases are a manual
download-and-attach for now.

### Signing tiers
Signing upgrades itself automatically based on which repository secrets
exist (Settings → Secrets and variables → Actions):

1. **No secrets (default):** ad-hoc signature. The app runs fine when built
   locally, but a *downloaded* DMG trips Gatekeeper ("unidentified
   developer") — users must right-click → Open once.
2. **Developer ID certificate** (requires the $99/yr Apple Developer
   Program). Secrets:
   - `MACOS_CERTIFICATE_P12` — the *Developer ID Application* certificate +
     private key exported from Keychain Access as a `.p12`, then
     `base64 -i DeveloperID.p12 | pbcopy`.
   - `MACOS_CERTIFICATE_PASSWORD` — the `.p12` password.
   CI imports the cert into a throwaway keychain and signs with the hardened
   runtime, a secure timestamp, and
   `trunk/mac/Build/Mac/BumpTop.entitlements` (Apple Events, so the Finder
   automation keeps working under the hardened runtime).
3. **Notarization** (recommended on top of tier 2 — this is what makes the
   DMG double-clickable for everyone). Create an App Store Connect API key
   (App Store Connect → Users and Access → Integrations → App Store Connect
   API, role: Developer) and add:
   - `NOTARY_KEY_ID` — the key ID.
   - `NOTARY_ISSUER_ID` — the issuer ID shown on the same page.
   - `NOTARY_KEY_P8` — the contents of the downloaded `AuthKey_XXXX.p8`.
   CI submits the DMG with `notarytool --wait`, staples the ticket, and
   verifies with `spctl`.

### Signing locally
`package_dmg.sh` honors `CODESIGN_IDENTITY`:

```bash
CODESIGN_IDENTITY="Developer ID Application: Your Name (TEAMID)" ./trunk/mac/package_dmg.sh
# then, to notarize by hand:
xcrun notarytool submit dist/BumpTop.dmg --wait --keychain-profile <profile>
xcrun stapler staple dist/BumpTop.dmg
```
Unset, it ad-hoc signs (`--sign -`), which is fine for local use.

### Ogre flags that matter
- `OGRE_NODELESS_POSITIONING=TRUE` — restores Camera/Light setPosition/lookAt
  (the codebase uses Ogre 1.7-style nodeless transforms everywhere).
- `OGRE_RESOURCEMANAGER_STRICT=0` — legacy resource lookup; the app loads
  textures by absolute path.
- Static GL render system (legacy OpenGL runs fine on Apple Silicon).
- **Vendored patch**: `trunk/mac/ogre-14-render-order.patch`
  (`#ManuallySpecifyingRenderOrder`) touches `OgreMain/include/OgreRenderable.h`
  + `OgreMain/src/OgreRenderQueueSortingGrouping.cpp`: re-adds BumpTop's
  setToRenderBefore/After render-order ties used for labels/highlights over
  icons. Porting note for anyone re-deriving it from the vendored 1.7.3 tree:
  Ogre renamed `DepthSortDescendingLess` to `DistanceSortDescendingLess` and
  moved it from OgreRenderQueueSortingGrouping.h into the .cpp, so a literal
  1.7.3 diff will not apply.

## What was ported (see git log for detail)
- Legacy Xcode 3 project → CMake/Ninja (`trunk/mac/CMakeLists.txt`); all
  sources compile as Objective-C++ with `BumpTop/port_prefix.h` force-included.
- Qt 4.6 → Qt 6.11 (Widgets split, Core5Compat QRegExp, QHttp →
  QNetworkAccessManager, qSort shim, AUTOMOC instead of checked-in moc files).
- Ogre 1.7 → 14.4 (overlay system registration, static plugin install in
  `BumpTopApp::createRootNode`, SharedPtr/AxisAlignedBox/Overlay API updates,
  `Resource::Listener::loadingComplete` rename — this one mattered: textures
  never applied without it).
- Bullet 2.78 → brew Bullet 3 (API compatible); protobuf gencode is produced
  at build time from `trunk/mac/BumpTop/protoc/AllMessages.proto` with
  whatever protoc is installed (generated headers hard-error on any version
  mismatch, so committing gencode broke every brew protobuf bump). The build
  post-processes the header to strip `final`/`PROTOBUF_FINAL` — the app
  inherits from the generated messages (see
  `trunk/mac/cmake/unfinal_protobuf_header.cmake`).
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
(as of 2026-08-18; done: parity ~1.9%, retina/label typography, gridded-pile
close button, sticky-note crashes + editing polish, animation smoothness
(idle-gate fix), branded DMG, NaN self-heal)

- Quick Look zoom-from rect fixed (device-px→Cocoa-points in
  QuickLookSnowLeopard.mm) but not yet user-verified — press space on a
  selected item and check the panel zooms from it.
- Bullet NaN root cause still unknown: the two-layer self-heal
  (PhysicsActorMotionState quarantine + DISABLE_SIMULATION rescue in
  Physics::stepSimulation) makes it harmless and logs `[nan] ...` with the
  poisoned actor's path — if it recurs, that log line is the lead.
- Lasso overlay churns materials while a drag grows it
  (Lasso::updateLassoMaterial recreates the QPainter texture; Ogre spams
  "force-disabling 'lighting'" warnings, each an unbuffered stderr write).
  Likely the residual lasso-drag jank; cap texture size / reuse material.
- Drag "stuck for half a second" report (DampedSpringMouseHandler): may
  already be cured by the idle-gate fix; if it recurs, run with
  BUMPTOP_PROFILE=1 (and BUMPTOP_PROFILE_ALL=1 for per-tick logs) and look
  for tick gaps / rendered=0 stretches in the newest stderr-*.txt.
- The New Items Pile is an always-present empty pile parked at room center;
  double-clicking it opens an empty grid — confusing, maybe hide it until
  it has members.
- Pro-license gating still active: more than 2 sticky notes requires
  `ProAuthorization` (server long dead) — consider unlocking in the port.
- A second BumpTop window shows in Mission Control previews occasionally
  (the 0x0 helper window) — cosmetic.
- Legacy deprecations still in use deliberately: OpenGL, NSOpenGLContext,
  QLThumbnailImageCreate, LSSharedFileList (login items), AppleScript Finder
  automation (may prompt for permission on first run; the DMG packager's
  Finder-layout scripting needs Automation permission too).
- `.port-deps/` is gitignored, but the Ogre patch is vendored at
  `trunk/mac/ogre-14-render-order.patch`, so a fresh machine only needs the
  Building steps above.
- Frame time on a busy desktop is ~30-50ms (legacy GL at 4K retina);
  animations are smooth now but a deeper render-cost pass (or a Metal-era
  RenderSystem) would lift everything.

Repro/debug hooks (env vars): BUMPTOP_TEST_NOTE=N, BUMPTOP_TEST_PILE_GRID=1,
BUMPTOP_TEST_GROW=N, BUMPTOP_TEST_NAN=1, BUMPTOP_PROFILE=1,
BUMPTOP_PROFILE_ALL=1, BUMPTOP_NO_STENCIL=1, BUMPTOP_DEBUG_LABELS=1,
BUMPTOP_LABEL_SIZE=N, BUMPTOP_PARITY_* (see parity section). Logs land in
`~/Library/Application Support/BumpTop/stderr-*.txt`.
