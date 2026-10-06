#!/usr/bin/env python3
"""Generate airShot's native vector camera and annotation-pencil icon.

Usage: make-icon.py output.hvif [preview.png]. The open camera silhouette,
blue lens and orange pencil stay legible at small Deskbar sizes.
"""
from pathlib import Path
import sys
import hvif

C = hvif.hex_color
styles, paths, shapes = [], [], []


def solid(color):
    styles.append({'color': C(color)})
    return len(styles) - 1


def gradient(start, end, colors):
    styles.append(hvif.linear_gradient(start, end,
        [(i / (len(colors) - 1), C(color)) for i, color in enumerate(colors)]))
    return len(styles) - 1


def shape(style, points, detail=False):
    paths.append({'closed': True, 'points': points})
    item = {'style': style, 'paths': [len(paths) - 1]}
    if detail:
        item['lod'] = (0.5, 4.0)
    shapes.append(item)


def ellipse(style, cx, cy, rx, ry, detail=False):
    k = 0.55228475
    shape(style, [
        ((cx + rx, cy), (cx + rx, cy - k * ry), (cx + rx, cy + k * ry)),
        ((cx, cy + ry), (cx + k * rx, cy + ry), (cx - k * rx, cy + ry)),
        ((cx - rx, cy), (cx - rx, cy + k * ry), (cx - rx, cy - k * ry)),
        ((cx, cy - ry), (cx - k * rx, cy - ry), (cx + k * rx, cy - ry)),
    ], detail)


outline = solid('233e54')
edge = gradient((8, 37), (54, 52), ['557788', '304f68'])
body = gradient((12, 16), (40, 49), ['f4f6ed', 'b5cbd0'])
top = solid('e7eee6')
ring = gradient((19, 22), (38, 46), ['5d8098', '244660'])
glass = gradient((22, 24), (36, 42), ['74d9e5', '288fba', '235891'])
shine = solid('d1fcfa')
orange = gradient((43, 27), (56, 50), ['ffcd6b', 'ec8e31'])
light_orange = solid('ffe2a0')
wood = solid('f8e4b8')
white = solid('ffffff')

# Camera body and its bevel; no enclosing app tile.
shape(outline, [(5, 18), (16, 15), (20, 7), (35, 5), (41, 11), (53, 10),
                (59, 15), (59, 45), (53, 51), (10, 55), (4, 49)])
shape(edge, [(8, 20), (54, 13), (56, 17), (56, 44), (51, 48), (11, 52), (7, 48)])
shape(body, [(7, 20), (19, 18), (23, 10), (34, 8), (40, 15), (53, 13),
             (53, 45), (7, 50)])
shape(top, [(8, 20), (20, 18), (24, 11), (34, 10), (37, 15), (22, 19), (8, 22)], True)
shape(outline, [(42, 19), (49, 18), (49, 23), (42, 24)])
shape(shine, [(43, 20), (48, 19), (48, 22), (43, 23)], True)

ellipse(outline, 29, 33, 15, 16)
ellipse(ring, 29, 32.5, 12.5, 13.5)
ellipse(glass, 29, 32.5, 9, 10)
shape(shine, [(24, 27), (28, 25), (30, 25), (24, 32), (22, 32)], True)

# A diagonal annotation pencil, with a strong tip at the lower right.
shape(outline, [(39, 45), (50, 27), (54, 25), (62, 31), (62, 35),
                (50, 54), (36, 60)])
shape(orange, [(41, 46), (51, 29), (59, 34), (48, 52)])
shape(light_orange, [(42, 44), (51, 29), (54, 31), (44, 47)], True)
shape(wood, [(41, 48), (47, 52), (39, 56)])
shape(outline, [(39, 53), (42, 55), (38, 57)])
shape(white, [(51, 28), (54, 27), (60, 32), (59, 34)])

icon = {'styles': styles, 'paths': paths, 'shapes': shapes}
if __name__ == '__main__':
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else Path('airshot-icon.hvif')
    data = hvif.encode(icon)
    hvif.decode(data)
    out.write_bytes(data)
    out.with_suffix('.svg').write_text(hvif.to_svg(icon))
    if len(sys.argv) > 2:
        hvif.preview(icon, 256).save(sys.argv[2])
    print(f'{out}: {len(data)} bytes, HVIF + SVG')
