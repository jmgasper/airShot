# Toolbar artwork

The toolbar and launcher use Font Awesome Free **6.7.2** by Fonticons, Inc.,
Copyright 2024, under **CC BY 4.0**. The original SVGs and license are in
`fontawesome/`. Source: https://github.com/FortAwesome/Font-Awesome/tree/6.7.2/svgs
and https://fontawesome.com/license/free.

The SVGs are converted into embedded Haiku Vector Icon Format data by
`tools/make-tool-icons.py`. This centers the visible artwork in a 64-unit
canvas with an 8-unit margin. Haiku renders the vector masks at the UI's
requested size; airShot tints them with the panel text color. Normal builds
need neither Python dependencies nor a system Font Awesome installation.

`IconButton` keeps native button frames, labels, sizing and interactions, but
draws the icon with `B_OP_ALPHA`. The standard control look uses `B_OP_OVER`,
which drops partial transparency on the workstation: antialiasing disappears,
the pixelation grid fills in, and disabled icons look enabled. Always check
the actual launcher and toolbar as well as the exported icon masks.

| UI icon | Source / adaptation |
| --- | --- |
| Select | solid arrow-pointer |
| Arrow | solid arrow-right, rotated 45 degrees counterclockwise |
| Line | original filled diagonal stroke |
| Rectangle | regular square, compressed vertically |
| Ellipse | regular circle, compressed vertically |
| Pen | solid pen |
| Highlighter | solid highlighter |
| Text | solid t |
| Counter | solid 1 cut out of an original circle |
| Blur | original 3-by-3 grid with graduated opacity |
| Crop | solid crop-simple |
| Undo / redo | solid rotate-left / rotate-right |
| Copy | regular copy |
| Save | solid floppy-disk |
| Full screen | solid desktop |
| Window | regular window-maximize |
| Region | solid expand |
| Settings | solid gear |

All adaptations and original shapes are by air/OS contributors (2026).
The application / Deskbar badge remains the original airShot branding.

To regenerate after an artwork change, install Python `fonttools` and run
`make tool-icons`. Commit the resulting `src/editor/ToolIconData.h` with the
source changes. No network access is needed for regeneration.

For visual checks on Haiku, run `make icon-dump`, then
`build-haiku/icon_dump /tmp/icons 20` in an existing output directory. The
last argument selects the rendered size (default 48). Check at 16, 20, 24,
32 and 48 pixels, on both light and dark backgrounds.
