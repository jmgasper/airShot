#!/usr/bin/env python3
"""Build airShot's application icon: a blue rounded tile with a white
selection frame (corner brackets) and an orange annotation arrow, readable
from 16 px up.

    python3 tools/make-icon.py resources/branding/airshot-icon.hvif [preview.png]
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hvif  # noqa: E402

C = hvif.hex_color
K = 0.5523


def curve(point, pin, pout):
    return (point, pin, pout)


def rounded_rect(left, top, right, bottom, radius):
    k = K * radius
    l, t, r, b, rad = left, top, right, bottom, radius
    return {'closed': True, 'points': [
        curve((l + rad, t), (l + rad - k, t), (l + rad, t)),
        curve((r - rad, t), (r - rad, t), (r - rad + k, t)),
        curve((r, t + rad), (r, t + rad - k), (r, t + rad)),
        curve((r, b - rad), (r, b - rad), (r, b - rad + k)),
        curve((r - rad, b), (r - rad + k, b), (r - rad, b)),
        curve((l + rad, b), (l + rad, b), (l + rad - k, b)),
        curve((l, b - rad), (l, b - rad + k), (l, b - rad)),
        curve((l, t + rad), (l, t + rad), (l, t + rad - k)),
    ]}


def bracket(cx, cy, dx, dy, length):
    """An L shape with its corner at (cx, cy), arms towards dx and dy."""
    return {'closed': False, 'points': [
        (cx + dx * length, cy), (cx, cy), (cx, cy + dy * length)]}


def arrow(start, end, shaft, head_length, head_width):
    """A filled arrow polygon from start to end."""
    (ax, ay), (bx, by) = start, end
    dx, dy = bx - ax, by - ay
    length = math.hypot(dx, dy)
    ux, uy = dx / length, dy / length
    px, py = -uy, ux  # perpendicular
    hx, hy = bx - ux * head_length, by - uy * head_length
    s = shaft / 2.0
    w = head_width / 2.0
    return {'closed': True, 'points': [
        (ax + px * s, ay + py * s),
        (hx + px * s, hy + py * s),
        (hx + px * w, hy + py * w),
        (bx, by),
        (hx - px * w, hy - py * w),
        (hx - px * s, hy - py * s),
        (ax - px * s, ay - py * s),
    ]}


# ------------------------------------------------------------------ styles
TILE = hvif.linear_gradient((32, 4), (32, 60), [
    (0.0, C('5cb4ff')), (0.55, C('2d7fe6')), (1.0, C('1b55b8'))])
TILE_OUTLINE = {'color': C('0d3577')}
GLOSS = hvif.linear_gradient((32, 5), (32, 30), [
    (0.0, (255, 255, 255, 95)), (1.0, (255, 255, 255, 0))])
FRAME = {'color': C('ffffff')}
FRAME_SHADOW = {'color': (0, 30, 80, 70)}
ARROW = hvif.linear_gradient((20, 46), (48, 18), [
    (0.0, C('ff8a1e')), (1.0, C('ffc247'))])
ARROW_OUTLINE = {'color': C('7a3a00')}
SHADOW = hvif.radial_gradient((32, 60), 26, [
    (0.0, (10, 20, 40, 100)), (0.7, (10, 20, 40, 35)), (1.0, (10, 20, 40, 0))], ratio=0.14)

style_names = ['tile', 'tileOutline', 'gloss', 'frame', 'frameShadow', 'arrow', 'arrowOutline',
               'shadow']
styles = [TILE, TILE_OUTLINE, GLOSS, FRAME, FRAME_SHADOW, ARROW, ARROW_OUTLINE, SHADOW]
S = {name: index for index, name in enumerate(style_names)}

# ------------------------------------------------------------------ paths
tile = rounded_rect(5, 5, 59, 59, 11)
gloss = rounded_rect(7, 7, 57, 30, 9)
arm = 10
brackets = [
    bracket(13, 13, 1, 1, arm),
    bracket(51, 13, -1, 1, arm),
    bracket(51, 51, -1, -1, arm),
    bracket(13, 51, 1, -1, arm),
]
annotation = arrow((20, 45), (46, 19), 6.5, 15, 17)
shadow = {'closed': True, 'points': [
    curve((32 + 1, 60), (32 + 1, 60 - K), (32 + 1, 60 + K)),
    curve((32, 61), (32 + K, 61), (32 - K, 61)),
    curve((31, 60), (31, 60 + K), (31, 60 - K)),
    curve((32, 59), (32 - K, 59), (32 + K, 59)),
]}

path_names = ['tile', 'gloss', 'b0', 'b1', 'b2', 'b3', 'arrow', 'shadow']
paths = [tile, gloss] + brackets + [annotation, shadow]
P = {name: index for index, name in enumerate(path_names)}


def shape(style, *names, **extra):
    result = {'style': S[style], 'paths': [P[n] for n in names]}
    result.update(extra)
    return result


stroke_wide = {'type': 'stroke', 'width': 4.5, 'join': 2, 'cap': 1, 'miter': 4}
stroke_narrow = {'type': 'stroke', 'width': 5.5, 'join': 2, 'cap': 1, 'miter': 4}
stroke_shadow = {'type': 'stroke', 'width': 6.5, 'join': 2, 'cap': 1, 'miter': 4}
contour_large = {'type': 'contour', 'width': 2.5, 'join': 2, 'miter': 4}
contour_small = {'type': 'contour', 'width': 2, 'join': 2, 'miter': 4}
contour_arrow = {'type': 'contour', 'width': 2, 'join': 2, 'miter': 4}

SMALL = {'lod': (0.0, 0.5)}
LARGE = {'lod': (0.5, 4.0)}
DETAIL = {'lod': (0.95, 4.0)}

shapes = [
    shape('shadow', 'shadow', matrix=[27.0, 0.0, 0.0, 3.5, 32.0 - 32.0 * 27.0, 60.0 - 60.0 * 3.5],
          **LARGE),
    shape('tileOutline', 'tile', transformers=[contour_large], **LARGE),
    shape('tileOutline', 'tile', transformers=[contour_small], **SMALL),
    shape('tile', 'tile'),
    shape('gloss', 'gloss', **DETAIL),
    shape('frameShadow', 'b0', 'b1', 'b2', 'b3', transformers=[stroke_shadow], **LARGE),
    shape('frame', 'b0', 'b1', 'b2', 'b3', transformers=[stroke_wide], **LARGE),
    shape('frame', 'b0', 'b1', 'b2', 'b3', transformers=[stroke_narrow], **SMALL),
    shape('arrowOutline', 'arrow', transformers=[contour_arrow], **LARGE),
    shape('arrow', 'arrow'),
]

icon = {'styles': styles, 'paths': paths, 'shapes': shapes}

if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else 'airshot-icon.hvif'
    data = hvif.encode(icon)
    hvif.decode(data)
    with open(out, 'wb') as f:
        f.write(data)
    print('%s: %d bytes' % (out, len(data)))
    svg = out.rsplit('.', 1)[0] + '.svg'
    with open(svg, 'w') as f:
        f.write(hvif.to_svg(icon))
    print('%s: SVG source' % svg)
    if len(sys.argv) > 2:
        image = hvif.preview(icon, 256)
        image.save(sys.argv[2])
        print('%s: preview' % sys.argv[2])
