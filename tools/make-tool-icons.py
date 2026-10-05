#!/usr/bin/env python3
"""Generate the embedded HVIF toolbar icons from the checked-in SVG artwork.

Only regeneration needs fontTools (pip install fonttools); normal Haiku builds
use ToolIconData.h directly. Font Awesome Free 6.7.2 artwork is CC BY 4.0.
See resources/icons/README.md for attribution and the icon mapping.
"""
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from fontTools.pens.basePen import BasePen
from fontTools.pens.boundsPen import BoundsPen
from fontTools.svgLib.path import parse_path

import hvif

ROOT = Path(__file__).resolve().parent.parent
SOURCES = ROOT / 'resources/icons/fontawesome'


class IconPen(BasePen):
    """Convert SVG contours to HVIF points with incoming/outgoing handles."""
    def __init__(self, transform):
        super().__init__(None)
        self.transform = transform
        self.paths = []
        self.points = []

    def _moveTo(self, point):
        point = self.transform(*point)
        self.points = [[point, point, point]]

    def _lineTo(self, point):
        point = self.transform(*point)
        self.points.append([point, point, point])

    def _curveToOne(self, first, second, end):
        first, second, end = [self.transform(*p) for p in (first, second, end)]
        self.points[-1][2] = first
        self.points.append([end, second, end])

    def _closePath(self):
        if self.points[-1][0] == self.points[0][0]:
            self.points[0][1] = self.points.pop()[1]
        self.paths.append({'closed': True, 'points': self.points})
        self.points = []

    def _endPath(self):
        raise ValueError('Toolbar artwork must use closed, filled contours')


def svg_paths(name, width=48, height=48, angle=0):
    root = ET.parse(SOURCES / (name + '.svg')).getroot()
    data = [p.attrib['d'] for p in root.findall('{http://www.w3.org/2000/svg}path')]
    bounds = BoundsPen(None)
    for path in data:
        parse_path(path, bounds)
    left, top, right, bottom = bounds.bounds
    # Centre on the actual ink; SVG viewBoxes have inconsistent whitespace.
    scale = min(width / (right - left), height / (bottom - top))
    cosine, sine = math.cos(angle), math.sin(angle)

    def transform(x, y):
        x, y = (x - (left + right) / 2) * scale, (y - (top + bottom) / 2) * scale
        return (32 + x * cosine - y * sine, 32 + x * sine + y * cosine)

    pen = IconPen(transform)
    for path in data:
        parse_path(path, pen)
    return pen.paths


def artwork(paths):
    return {'styles': [{'color': (0, 0, 0, 255)}], 'paths': paths,
            'shapes': [{'style': 0, 'paths': list(range(len(paths)))}]}


def polygon(points):
    return {'closed': True, 'points': points}


def stretch(paths, x=1, y=1):
    for path in paths:
        path['points'] = [tuple((32 + (px - 32) * x, 32 + (py - 32) * y)
                               for px, py in hvif.normalize_point(point))
                          for point in path['points']]
    return paths


def counter():
    # A true cut-out numeral stays transparent over selected/disabled buttons.
    k = 24 * 0.55228475
    circle = {'closed': True, 'points': [
        ((56, 32), (56, 32 - k), (56, 32 + k)),
        ((32, 56), (32 + k, 56), (32 - k, 56)),
        ((8, 32), (8, 32 + k), (8, 32 - k)),
        ((32, 8), (32 - k, 8), (32 + k, 8)),
    ]}
    digit = svg_paths('solid-1', 22, 30)
    # Force the numeral's winding opposite to the outer circle.
    for path in digit:
        points = path['points']
        area = sum(p[0][0] * q[0][1] - q[0][0] * p[0][1]
                   for p, q in zip(points, points[1:] + points[:1]))
        if area > 0:
            path['points'] = [[p, pout, pin] for p, pin, pout in reversed(points)]
    return artwork([circle] + digit)


def blur():
    icon = {'styles': [], 'paths': [], 'shapes': []}
    # A regular pixel grid with graduated opacity reads as pixelation at 16 px.
    for i, alpha in enumerate((95, 155, 215, 155, 215, 255, 215, 255, 255)):
        x, y = 8 + i % 3 * 17, 8 + i // 3 * 17
        icon['styles'].append({'color': (0, 0, 0, alpha)})
        icon['paths'].append(polygon([(x, y), (x + 14, y), (x + 14, y + 14), (x, y + 14)]))
        icon['shapes'].append({'style': i, 'paths': [i]})
    return icon


def make_icons():
    return [
        ('Select', artwork(svg_paths('solid-arrow-pointer'))),
        ('Arrow', artwork(svg_paths('solid-arrow-right', angle=-math.pi / 4))),
        ('Line', artwork([polygon([(11, 48), (48, 11), (53, 16), (16, 53)])])),
        ('Rectangle', artwork(stretch(svg_paths('regular-square'), y=0.75))),
        ('Ellipse', artwork(stretch(svg_paths('regular-circle'), y=0.75))),
        ('Pen', artwork(svg_paths('solid-pen'))),
        ('Highlighter', artwork(svg_paths('solid-highlighter'))),
        ('Text', artwork(svg_paths('solid-t', 42, 46))),
        ('Counter', counter()),
        ('Blur', blur()),
        ('Crop', artwork(svg_paths('solid-crop-simple'))),
        ('Undo', artwork(svg_paths('solid-rotate-left'))),
        ('Redo', artwork(svg_paths('solid-rotate-right'))),
        ('Copy', artwork(svg_paths('regular-copy'))),
        ('Save', artwork(svg_paths('solid-floppy-disk'))),
        ('FullScreen', artwork(svg_paths('solid-desktop'))),
        ('Window', artwork(svg_paths('regular-window-maximize'))),
        ('Region', artwork(svg_paths('solid-expand'))),
        ('Settings', artwork(svg_paths('solid-gear'))),
    ]


def main():
    icons = make_icons()
    lines = ['// Generated by tools/make-tool-icons.py; do not edit.',
             '// Font Awesome Free 6.7.2, Copyright 2024 Fonticons, Inc. (CC BY 4.0).',
             '// Adaptations and original shapes: air/OS contributors.',
             '// See resources/icons/README.md and fontawesome/LICENSE.txt.',
             '#pragma once', '', 'namespace airshot {', 'namespace {', '',
             'struct VectorIcon {', '\tconst uint8* data;', '\tsize_t size;', '};', '']
    total = 0
    for name, icon in icons:
        data = hvif.encode(icon)
        decoded = hvif.decode(data)
        assert len(decoded['paths']) == len(icon['paths'])
        total += len(data)
        lines.append('const uint8 k%sData[] = {' % name)
        for start in range(0, len(data), 12):
            lines.append('\t' + ', '.join('0x%02x' % b for b in data[start:start + 12]) + ',')
        lines.extend(['};', ''])
    for table, entries in [('Tool', icons[:11]), ('Action', icons[11:])]:
        lines.append('const VectorIcon k%sIcons[] = {' % table)
        for name, _ in entries:
            lines.append('\t{k%sData, sizeof(k%sData)},' % (name, name))
        lines.extend(['};', ''])
    lines.extend(['}  // namespace', '}  // namespace airshot', ''])
    (ROOT / 'src/editor/ToolIconData.h').write_text('\n'.join(lines))
    print('Generated %d icons (%d bytes of HVIF)' % (len(icons), total))


if __name__ == '__main__':
    main()
