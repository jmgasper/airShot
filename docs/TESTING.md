# Testing airShot

## Workstation

airShot is built and tested on the X399 Haiku workstation (192.168.1.244):

- `tools/ws.sh '<command>'` runs a command there over SSH.
- `tools/sync-build.sh [target]` syncs the tree to `/boot/home/airshot` and
  runs `make` there, printing errors only.
- `tools/install-dev.sh [--run]` installs the filter into
  `~/config/non-packaged/add-ons/input_server/filters/airShot` and the app into
  `~/config/non-packaged/apps/airShot`, optionally starting it.
- `tools/sync-build.sh package` builds the hpkg in `artifacts/`;
  `pkgman install -y <hpkg>` installs it (bump the version first when
  reinstalling).

The workstation's VNC server (port 5900) injects input through the
`InputEventInjector` device, so keystrokes sent over VNC pass through
`input_server` and therefore through the shortcut filter, which makes the
shortcuts testable remotely:

```
python3 tools/vnc.py shot out.png --scale 0.5     # screenshot
python3 tools/vnc.py key 0xFF61                    # Print Screen
python3 tools/vnc.py click X Y / drag X0 Y0 X1 Y1 / type "text"
```

The NanoKVM at 192.168.1.22 can send real USB keyboard events as a second
path (`/mnt/HaikuWork/x399/tools/kvm.py`).

## Unit tests

`make BUILD=build-host check-host` on Linux (or `make check` on Haiku) runs
`tests/HotKeyTests.cpp`: shortcut matching, labels and key names.

## Manual checklist

1. Print Screen with nothing running: airShot starts and opens the editor
   with the full screen.
2. Shift+Print Screen: overlay in window mode; hovering moves the highlight
   with an animation; clicking captures the window (tab included, corners
   transparent); dragging switches to a region.
3. Ctrl+Print Screen: crosshair, drag a region, adjust with handles, Enter.
4. Editor: every tool, undo/redo, zoom (Alt+wheel), Save (notification and
   file in the configured folder), Copy (paste into another app).
5. Settings: record a new shortcut (the old one must not fire while
   recording), change the save folder, toggle the Deskbar icon.
6. Deskbar icon menu: captures, open, settings, quit.
7. `airShot picture.png` opens the file in the editor.
