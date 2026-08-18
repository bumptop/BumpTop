#!/bin/zsh
# Desktop parity test: capture the real Finder desktop, run BumpTop in parity
# mode (BUMPTOP_PARITY=1: top-down camera, uniform lighting, wallpaper floor),
# capture BumpTop's window, and pixel-diff the two.
#
# usage: parity_run.sh [output-dir] [path-to-BumpTop.app]
set -e
SCRIPT_DIR=${0:a:h}
OUT=${1:-/tmp/bt-parity}
APP=${2:-$SCRIPT_DIR/../../../build/BumpTop.app}
SUPPORT="$HOME/Library/Application Support/BumpTop"

# Homebrew's Pillow lives in a keg; make it importable.
export PYTHONPATH="$(ls -d /opt/homebrew/opt/pillow/lib/python3.*/site-packages | tail -1)"
PYTHON=/opt/homebrew/bin/python3

mkdir -p "$OUT"
pkill -x BumpTop 2>/dev/null || true
sleep 1

echo "== capturing wallpaper + Finder icon layer"
WALLPAPER_ID=$(swift "$SCRIPT_DIR/capture_desktop.swift" wallpaper)
FINDER_ID=$(swift "$SCRIPT_DIR/capture_desktop.swift" finder)

if screencapture -x -o -l $WALLPAPER_ID "$OUT/wallpaper.png"; then
  cp "$OUT/wallpaper.png" "$SUPPORT/parity_wallpaper.png"
else
  echo "   (wallpaper window not capturable; reusing previous capture)"
  cp "$SUPPORT/parity_wallpaper.png" "$OUT/wallpaper.png"
fi
if screencapture -x -o -l $FINDER_ID "$OUT/finder_icons.png"; then
  cp "$OUT/finder_icons.png" "$SUPPORT/parity_finder_icons.png"
else
  echo "   (finder icon window not capturable; reusing previous capture)"
  cp "$SUPPORT/parity_finder_icons.png" "$OUT/finder_icons.png"
fi

echo "== compositing reference desktop"
$PYTHON - "$OUT" <<'EOF'
import sys
from PIL import Image
out = sys.argv[1]
wallpaper = Image.open(out + "/wallpaper.png").convert("RGBA")
icons = Image.open(out + "/finder_icons.png").convert("RGBA")
w = min(wallpaper.width, icons.width)
h = min(wallpaper.height, icons.height)
wallpaper = wallpaper.crop((0, wallpaper.height - h, w, wallpaper.height))
icons = icons.crop((0, icons.height - h, w, icons.height))
Image.alpha_composite(wallpaper, icons).convert("RGB").save(out + "/reference.png")
print("reference.png", w, h)
EOF


cp "$OUT/wallpaper.png" "$SUPPORT/parity_wallpaper.png"

# Clean scene state so item positions are re-imported from Finder, and drop
# stale cached floor textures.
rm -f "$SUPPORT/Room.bump"
rm -rf "$SUPPORT/backgrounds"

echo "== launching BumpTop in parity mode"
FINDER_ICON_SIZE=$(defaults read com.apple.finder DesktopViewSettings 2>/dev/null | awk '/iconSize/ {gsub(/;/,""); print $3}')
BUMPTOP_PARITY=1 BUMPTOP_PARITY_ICON_SIZE=${FINDER_ICON_SIZE:-64} "$APP/Contents/MacOS/BumpTop" &
sleep 15

echo "== capturing BumpTop render"
# Prefer the framebuffer dump BumpTop writes itself in parity mode (immune to
# Space/window-server capture issues); fall back to a window capture.
rm -f "$SUPPORT/parity_render.png"
for i in $(seq 1 30); do
  [ -f "$SUPPORT/parity_render.png" ] && break
  sleep 1
done
if [ -f "$SUPPORT/parity_render.png" ]; then
  sleep 1  # let the write finish
  cp "$SUPPORT/parity_render.png" "$OUT/bumptop.png"
else
  BT_ID=$(swift "$SCRIPT_DIR/capture_desktop.swift" bumptop)
  screencapture -x -o -l $BT_ID "$OUT/bumptop.png"
fi

echo "== diffing"
$PYTHON "$SCRIPT_DIR/diff_images.py" "$OUT/reference.png" "$OUT/bumptop.png" \
        "$OUT/heatmap.png" "$OUT/sidebyside.png"
