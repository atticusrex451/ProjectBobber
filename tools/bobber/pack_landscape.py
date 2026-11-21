"""Pack the indexed landscape for RGB565 firmware, using Pillow.

Run from any directory: python3 tools/bobber/pack_landscape.py lake
"""
from argparse import ArgumentParser
from itertools import groupby
from pathlib import Path

from PIL import Image

parser = ArgumentParser(description=__doc__)
parser.add_argument("region", choices=("bay", "lake", "grotto"))
region = parser.parse_args().region
sources = {"bay": "chincoteague.png", "lake": "blue-mountain-lake.png", "grotto": "thurls-grotto.png"}
source = Path(__file__).with_name(sources[region])
output = Path(__file__).resolve().parents[2] / f"bobber_{region}_background.h"
prefix = f"bobber{region.title()}"
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
    f"#pragma once\n\n// Generated from tools/bobber/{source.name} by pack_landscape.py.\n"
    + array(prefix + "Palette", palette565)
    + array(prefix + "Rows", rows)
    + array(prefix + "Runs", runs)
)
