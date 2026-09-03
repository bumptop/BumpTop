#!/bin/zsh
# Builds BumpTop.app, bundles all non-system dependencies (Qt frameworks,
# protobuf/abseil/bullet dylibs) via macdeployqt, ad-hoc signs it, and wraps
# it in a compressed DMG.
#
# usage: package_dmg.sh [build-dir] [output-dir]
set -e
SCRIPT_DIR=${0:a:h}
REPO_ROOT=${SCRIPT_DIR:h:h}
BUILD=${1:-$REPO_ROOT/build}
OUT=${2:-$REPO_ROOT/dist}
QT_BIN=/opt/homebrew/opt/qt/bin

echo "== building"
/opt/homebrew/bin/cmake --build "$BUILD"

echo "== staging"
mkdir -p "$OUT"
rm -rf "$OUT/BumpTop.app" "$OUT/BumpTop.dmg"
cp -R "$BUILD/BumpTop.app" "$OUT/BumpTop.app"
# dev-run log dumps don't belong in a shipping bundle
rm -f "$OUT/BumpTop.app/Contents/Resources"/stdout-*.txt \
      "$OUT/BumpTop.app/Contents/Resources"/stderr-*.txt

echo "== bundling dependencies (macdeployqt)"
"$QT_BIN/macdeployqt" "$OUT/BumpTop.app"

echo "== signing (ad hoc)"
codesign --force --deep --sign - "$OUT/BumpTop.app"

echo "== creating DMG"
# Branded layout, ported from Build/Mac/create_dmg.sh: background art,
# app + /Applications alias side by side, sized window.
BACKGROUND="$SCRIPT_DIR/Build/Mac/dmg-background.png"
STAGING="$OUT/dmg-staging"
TMP_DMG="$OUT/pack.temp.dmg"
rm -rf "$STAGING" "$TMP_DMG"
# a stale mounted BumpTop volume would steal the /Volumes/BumpTop mount point
while [ -d "/Volumes/BumpTop" ]; do hdiutil detach "/Volumes/BumpTop" -force -quiet || break; done
mkdir -p "$STAGING"
cp -R "$OUT/BumpTop.app" "$STAGING/BumpTop.app"

APP_MB=$(du -sm "$STAGING" | awk '{print $1}')
hdiutil create -srcfolder "$STAGING" -volname BumpTop -fs HFS+ \
    -format UDRW -size $((APP_MB + 40))m "$TMP_DMG" -ov -quiet
ATTACH_OUT=$(hdiutil attach -readwrite -noverify -noautoopen "$TMP_DMG")
DEVICE=$(echo "$ATTACH_OUT" | egrep '^/dev/' | sed 1q | awk '{print $1}')
MOUNT=$(echo "$ATTACH_OUT" | egrep -o '/Volumes/.*$' | sed 1q)
[ -d "$MOUNT" ] || { echo "mount failed"; exit 1; }

mkdir -p "$MOUNT/.background"
cp "$BACKGROUND" "$MOUNT/.background/dmg-background.png"
ln -s /Applications "$MOUNT/Applications"

# Finder writes the .DS_Store carrying the view options. Needs Automation
# permission for Finder; layout is cosmetic, so a TCC denial is not fatal.
osascript <<'EOS' || echo "warning: Finder layout scripting failed; DMG will use default view"
tell application "Finder"
  tell disk "BumpTop"
    open
    set current view of container window to icon view
    set toolbar visible of container window to false
    set statusbar visible of container window to false
    set the bounds of container window to {400, 100, 992, 445}
    set theViewOptions to the icon view options of container window
    set arrangement of theViewOptions to not arranged
    set icon size of theViewOptions to 120
    set background picture of theViewOptions to file ".background:dmg-background.png"
    set position of item "BumpTop" of container window to {140, 165}
    set position of item "Applications" of container window to {453, 165}
    close
    open
    update without registering applications
    delay 3
    close
  end tell
end tell
EOS

sync
hdiutil detach "$DEVICE" -quiet
hdiutil convert "$TMP_DMG" -format UDZO -imagekey zlib-level=9 -o "$OUT/BumpTop.dmg" -ov -quiet
rm -f "$TMP_DMG"
rm -rf "$STAGING"
echo "wrote $OUT/BumpTop.dmg"
