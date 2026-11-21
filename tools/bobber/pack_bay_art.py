"""Pack the indexed Chincoteague painting for RGB565 firmware, using Pillow.

Run from any directory: python3 tools/bobber/pack_bay_art.py
"""
from itertools import groupby
from pathlib import Path

from PIL import Image

source = Path(__file__).with_name("chincoteague.png")
output = Path(__file__).resolve().parents[2] / "bobber_bay_background.h"
with Image.open(source) as painting:
    assert painting.mode == "P" and painting.size == (233, 233)
    pixels = list(painting.tobytes())
    palette = painting.getpalette()

num_colors = max(pixels) + 1
palette565 = [
    ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    for r, g, b in zip(palette[0::3], palette[1::3], palette[2::3])
][:num_colors]
rows, runs = [], []
for y in range(233):
    rows.append(len(runs))
    for color, group in groupby(pixels[y * 233 : (y + 1) * 233]):
        # High byte counts art pixels, low byte holds the palette index.
        runs.append((len(list(group)) << 8) | color)

rows.append(len(runs))
assert len(runs) <= 65535

def array(name, values):
    lines = [f"static const uint16_t {name}[] = {{"]
    for i in range(0, len(values), 16):
        lines.append("  " + ", ".join(f"0x{v:04x}" for v in values[i:i + 16]) + ",")
    return "\n".join(lines + ["};", ""])

output.write_text(
    "#pragma once\n\n// Generated from tools/bobber/chincoteague.png by pack_bay_art.py.\n"
    + array("bobberBayPalette", palette565)
    + array("bobberBayRows", rows)
    + array("bobberBayRuns", runs)
)
