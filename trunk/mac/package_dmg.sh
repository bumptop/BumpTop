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
hdiutil create -volname BumpTop -srcfolder "$OUT/BumpTop.app" -ov -format UDZO "$OUT/BumpTop.dmg"
echo "wrote $OUT/BumpTop.dmg"
