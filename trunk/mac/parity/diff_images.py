#!/usr/bin/env python3
"""Pixel-diff two parity captures (reference desktop vs BumpTop render).

Aligns the two images at their bottom-left corner (both run to the bottom of
the screen; BumpTop's window is shorter by the menu bar), then reports diff
statistics and writes a heatmap plus a side-by-side composite.
"""
import sys
from PIL import Image, ImageChops

def main():
    if len(sys.argv) != 5:
        print("usage: diff_images.py <reference.png> <bumptop.png> <heatmap.png> <sidebyside.png>")
        return 2
    ref = Image.open(sys.argv[1]).convert("RGB")
    bt = Image.open(sys.argv[2]).convert("RGB")

    w = min(ref.width, bt.width)
    h = min(ref.height, bt.height)
    # crop from the bottom edge
    ref_c = ref.crop((0, ref.height - h, w, ref.height))
    bt_c = bt.crop((0, bt.height - h, w, bt.height))

    diff = ImageChops.difference(ref_c, bt_c)
    gray = diff.convert("L")
    hist = gray.histogram()
    total = w * h
    changed = total - hist[0]
    # pixels differing by more than a small tolerance (antialiasing noise)
    significant = sum(hist[16:])
    mean = sum(i * c for i, c in enumerate(hist)) / total

    print(f"size compared: {w}x{h} = {total} px")
    print(f"pixels differing at all:   {changed:9d}  ({100.0*changed/total:6.2f}%)")
    print(f"pixels differing > 16/255: {significant:9d}  ({100.0*significant/total:6.2f}%)")
    print(f"mean abs diff: {mean:.2f}/255")

    heat = gray.point(lambda v: min(255, v * 4))
    heat_rgb = Image.merge("RGB", (heat, heat.point(lambda v: 0), heat.point(lambda v: 0)))
    base = ref_c.point(lambda v: v // 3)
    heatmap = ImageChops.add(base, heat_rgb)
    heatmap.save(sys.argv[3])

    side = Image.new("RGB", (w, h * 2 + 8), (30, 30, 30))
    side.paste(ref_c, (0, 0))
    side.paste(bt_c, (0, h + 8))
    side.save(sys.argv[4])
    return 0

if __name__ == "__main__":
    sys.exit(main())
