"""Draw the original Chincoteague fish sprites and pack opaque RGB565 spans.

Run with Pillow: python3 tools/bobber/paint_bay_fish.py
"""
from itertools import groupby
from pathlib import Path
from random import Random

from PIL import Image, ImageDraw

WIDTH, HEIGHT = 110, 46
OUT = Path(__file__).resolve().parents[2] / "bobber_bay_portraits.h"

# Body, dorsal fins and tail are drawn in native sprite pixels.
FISH = {
    "Croaker": (
        "#947152 #b9996d #dfc49b #866e55",
        [(12,25),(16,20),(23,14),(36,12),(59,14),(74,18),(86,20),(89,25),(72,28),(53,33),(33,34),(18,30)],
        [[(29,15),(37,5),(41,7),(48,14)],[(48,15),(54,11),(68,14),(76,19)]],
        [(84,20),(100,15),(101,20),(97,24),(101,29),(87,26)]),
    "Spot": (
        "#78826c #a8ad90 #dfd6ad #8f9474",
        [(17,25),(22,18),(32,12),(44,11),(63,15),(77,20),(81,24),(73,28),(55,34),(38,35),(25,32)],
        [[(31,14),(39,5),(43,6),(53,13)],[(53,14),(60,11),(71,15),(76,20)]],
        [(77,20),(95,14),(94,21),(90,24),(95,31),(77,27)]),
    "Weakfish": (
        "#676e7b #a1a09b #ddd0b6 #a99556",
        [(6,24),(14,18),(29,14),(43,13),(70,15),(89,20),(93,24),(80,27),(54,30),(29,31),(14,29),(8,27)],
        [[(27,15),(37,6),(42,7),(52,14)],[(54,15),(62,11),(77,13),(84,19)]],
        [(89,20),(106,14),(105,20),(101,24),(106,31),(90,26)]),
    "StripedBass": (
        "#516760 #8d9b8e #cbd2b6 #727d66",
        [(6,23),(12,18),(24,12),(40,9),(59,12),(75,16),(88,20),(92,25),(79,28),(53,34),(30,35),(14,31),(6,28)],
        [[(24,14),(33,3),(36,4),(40,7),(43,5),(48,12)],[(52,12),(59,7),(65,8),(78,17)]],
        [(87,20),(106,10),(104,18),(99,23),(106,33),(101,33),(88,27)]),
    "SpeckledTrout": (
        "#557b7a #8faaa1 #d4d9be #737e78",
        [(6,24),(13,18),(28,15),(43,13),(65,15),(85,18),(93,21),(94,25),(83,27),(60,31),(35,33),(17,30),(8,28)],
        [[(30,15),(39,6),(43,7),(51,14)],[(56,15),(63,11),(75,13),(83,19)]],
        [(88,20),(106,14),(103,20),(105,24),(105,30),(90,26)]),
    "BlackDrum": (
        "#484c51 #77797a #b9b5a2 #626165",
        [(8,27),(12,20),(22,11),(36,7),(51,8),(66,12),(80,18),(89,22),(90,26),(78,29),(59,35),(40,39),(25,37),(14,33)],
        [[(23,12),(30,2),(33,4),(36,3),(42,7),(49,9)],[(50,9),(56,7),(68,11),(78,18)]],
        [(84,21),(103,16),(105,23),(103,31),(85,27)]),
    "Cobia": (
        "#52463e #79695a #c6b79e #756451",
        [(4,24),(15,18),(31,15),(49,15),(67,17),(85,19),(93,22),(94,25),(80,27),(59,30),(32,30),(13,28),(4,26)],
        [[(36,16),(38,11),(40,16)],[(43,16),(45,12),(47,16)],[(49,16),(51,12),(53,16)],[(57,17),(62,7),(67,11),(79,17)]],
        [(90,21),(106,8),(105,16),(99,24),(107,36),(102,35),(91,26)]),
    "OldMan": (
        "#62392e #a86743 #e7bb84 #794632",
        [(4,28),(7,22),(15,15),(24,10),(36,7),(49,8),(68,12),(81,16),(93,21),(95,27),
         (82,31),(62,37),(40,40),(24,40),(13,37),(6,33)],
        [[(21,13),(27,4),(31,2),(35,4),(35,9),(40,5),(44,6),(49,9)],
         [(48,9),(57,6),(69,9),(83,17)]],
        [(88,20),(103,12),(108,16),(106,21),(102,23),(107,27),(108,33),(103,36),(90,29)]),
}

def sprite(name, colors, body, dorsals, tail):
    image = Image.new("RGB", (WIDTH, HEIGHT))
    draw = ImageDraw.Draw(image)
    ink = "#302c32"
    back, middle, belly, fin = colors.split()
    cheek = body[1][0] + 3
    chin = max(y for x, y in body if x <= cheek + 4)
    bottom = max(y for x, y in body)
    draw.polygon([(cheek+12,chin-4),(cheek+27,bottom+4),(cheek+34,bottom+1),
                  (cheek+27,chin-3)], fill=fin, outline=ink)
    for points in [tail, *dorsals]:
        draw.polygon(points, fill=fin, outline=ink)
        pivot = points[0]
        for x, y in points[1:-1]:
            draw.line((pivot,(x-1,y+1)), fill=back)
    draw.polygon(body, fill=middle, outline=ink)

    # The shoulder carries the light; the back and belly form long pixel clusters.
    for x in range(WIDTH):
        column = [y for y in range(HEIGHT) if image.getpixel((x,y)) == rgb(middle)]
        if len(column) < 4:
            continue
        top, low = column[0], column[-1]
        draw.line((x,top,x,top+(low-top)//4), fill=back)
        draw.line((x,low-(low-top)//4,x,low), fill=belly)
    rng = Random(name)
    for _ in range(135):
        x, y = rng.randrange(cheek+7,89), rng.randrange(9,38)
        color = image.getpixel((x,y))
        if color in (rgb(middle),rgb(back)):
            draw.point((x,y), fill=belly if color == rgb(back) else back)

    # Clip the markings to the silhouette and preserve its outline.
    marks = Image.new("RGB", image.size)
    m = ImageDraw.Draw(marks)
    if name == "Croaker":
        for x in range(35,81,6):
            m.line((x,16+(x-35)//9,x+4,21+(x-35)//9), fill="#6e533f", width=2)
    elif name == "Spot":
        m.ellipse((31,16,38,22), fill=ink)
        for x in range(46,78,5):
            m.line((x,15+(x-46)//8,x+3,19+(x-46)//8), fill=back)
    elif name in ("Weakfish", "SpeckledTrout"):
        for i in range(34 if name == "SpeckledTrout" else 25):
            x, y = rng.randrange(31,104), rng.randrange(9,24)
            m.rectangle((x,y,x+(i%3==0),y+(i%4==0)),
                        fill=ink if name == "SpeckledTrout" else "#795f68")
    elif name == "StripedBass":
        for y in (16,19,22,25,28,31):
            m.line([(27,y),(45,y-2),(66,y-1),(89,23+(y-22)//4)], fill="#4e6059")
    elif name == "BlackDrum":
        for x in (32,42,53,65,75):
            m.polygon([(x,7),(x+4,7),(x+7,33),(x+4,36)], fill="#52565a")
    elif name == "Cobia":
        m.polygon([(7,21),(41,19),(72,21),(94,23),(92,25),(68,25),(41,24),(9,25)], fill="#403b38")
    else:
        # The tail spot and stepped oyster-bed scar are readable at journal scale.
        m.ellipse((80,18,92,29), fill="#cc9361")
        m.ellipse((82,19,91,28), fill=ink)
        m.line([(47,12),(42,17),(39,23),(39,26),(35,31)], fill="#603b31", width=4)
        m.line([(47,12),(42,17),(39,23),(39,26),(35,31)], fill="#f7d5a1", width=2)
        m.line([(42,17),(39,17),(36,20)], fill="#e5ac79")
        for x, y in [(52,13),(58,15),(64,15),(69,18),(51,20),(60,21),(67,23)]:
            m.line([(x,y),(x+2,y+1),(x+5,y+1)], fill="#c68b57")
    for y in range(HEIGHT):
        for x in range(WIDTH):
            mark = marks.getpixel((x,y))
            if mark != (0,0,0) and image.getpixel((x,y)) not in ((0,0,0),rgb(ink)):
                image.putpixel((x,y),mark)

    ey, ex = body[1][1] + 2, cheek
    draw.ellipse((ex-2,ey-2,ex+3,ey+3),fill=belly,outline=ink)
    draw.rectangle((ex,ey,ex+1,ey+1),fill=ink)
    draw.point((ex,ey-1),fill="#ffebc5")
    draw.line((body[0][0]+1,body[0][1],cheek-2,body[0][1]+1),fill=ink)
    draw.line([(cheek+8,ey-2),(cheek+11,ey+4),(cheek+8,chin-1)],fill=back)
    draw.polygon([(cheek+13,chin-7),(cheek+28,chin-3),(cheek+20,chin),(cheek+12,chin-4)],fill=fin)
    draw.line((cheek+13,chin-6,cheek+24,chin-3),fill=back)
    if name == "OldMan":
        draw.line([(ex-4,ey-2),(ex,ey-4),(ex+6,ey-3)], fill=ink, width=2)
        draw.point((ex+1,ey-1), fill="#d9ab68")
    if name in ("BlackDrum","Croaker"):
        for x in range(body[0][0]+5,body[0][0]+12,2):
            draw.line((x,body[0][1]+4,x-1,body[0][1]+7),fill=ink)
    return image

def rgb(color):
    return tuple(bytes.fromhex(color[1:]))

def packed_color(color):
    r, g, b = color
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

def encode(image):
    spans = []
    for y in range(HEIGHT):
        x = 0
        for color, group in groupby((image.getpixel((x,y)) for x in range(WIDTH))):
            length = len(list(group))
            if color != (0,0,0):
                spans.append((x,y,length,packed_color(color)))
            x += length
    return spans

header = ["#pragma once", "", "// Generated by tools/bobber/paint_bay_fish.py.",
          "struct BobberPortraitSpan { uint8_t x, y, width; uint16_t color; };"]
for name, spec in FISH.items():
    spans = encode(sprite(name, *spec))
    header += ["", f"static const BobberPortraitSpan bobber{name}Portrait[] = {{"]
    entries = [f"{{{x},{y},{length},0x{color:04x}}}," for x,y,length,color in spans]
    header += ["  " + " ".join(entries[i:i+5]) for i in range(0,len(entries),5)]
    header += ["};"]
OUT.write_text("\n".join(header) + "\n")
