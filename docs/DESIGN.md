# airShot design notes

## Pieces

```
filter/airShotFilter.cpp   input_server add-on: watches key presses, sends
                           capture requests to the app (or launches it)
src/HotKey.*               shortcut model shared by app and filter (no Haiku
                           headers, unit tested on any platform)
src/Settings.*             flattened BMessage in ~/config/settings/airShot/settings,
                           read by both the app and the filter
src/App.*                  BApplication: capture orchestration, windows, Deskbar
src/capture/ScreenCapture  BScreen::GetBitmap, window list (private WindowInfo
                           API), cropping, transparent tab space
src/capture/OverlayWindow  full-screen picker over the frozen screen: window
                           hover highlight with animation, region selection
src/editor/Annotation      shape objects (arrow, line, rect, ellipse, pen,
                           highlighter, text, counter, blur) + blur filters
src/editor/CanvasView      zoomable canvas running the tools, undo/redo,
                           inline text editing
src/editor/EditorWindow    menus, BToolBar, colour swatches, save/copy
src/editor/Export          flatten to bitmap, PNG, clipboard, notifications
src/ui/MainWindow          launcher window
src/ui/SettingsWindow      settings; KeyCaptureControl records shortcuts
src/ui/DeskbarView         tray replicant (runs inside Deskbar's team)
```

## Capture flow

1. A request arrives (`kMsgCaptureFull/Window/Region`) from the filter, the
   Deskbar icon, the launcher or `ArgvReceived`. The app hides its own windows
   and waits 250 ms (plus the configured delay) so they are gone from the
   screen.
2. `ScreenCapture::GrabScreen` reads the whole screen. For a full-screen
   capture this is the result.
3. Otherwise `ScreenCapture::ListWindows` collects the visible windows of the
   workspace (front to back, our team excluded, desktop last) and an
   `OverlayWindow` (a `kWindowScreenWindow`, above everything) shows the
   frozen screen dimmed to 62.5%. The user picks a window or a region; the
   overlay reports a screen rectangle plus window details.
4. The rectangle is cropped out of the frozen bitmap. For a decorated window
   the owning application is asked for the tab frame (scripting `TabFrame`)
   and the space beside the tab is made transparent, as Haiku's Screenshot
   does.
5. Depending on the settings the bitmap opens in an `EditorWindow`, goes to
   the clipboard, or is saved as PNG (with a notification).

## The filter

`input_server` loads every add-on in `add-ons/input_server/filters`. Ours keeps
a `HotKeyDispatcher` looper that owns a copy of the settings, watches the
settings directory with `BPathMonitor`, and does the delivery, so
`Filter()` itself only compares key codes and never blocks the input thread.
A matching key press is swallowed (`B_SKIP_MESSAGE`), and so is its key-up.
Repeats (`be:key_repeat`) are ignored. While the settings window records a new
shortcut it sets `hotkeys_paused`, so the current shortcuts can be pressed
without firing.

When the app is not running, the filter launches it with `--full/--window/
--region`: command line arguments reach `ArgvReceived` before `ReadyToRun`,
which lets the app skip opening its launcher window.

## The overlay

The overlay keeps two bitmaps: the screen and a dimmed copy (one shift-and-
mask per pixel). Drawing paints the dimmed copy for the update rectangle, the
bright original inside the selection, then the frame (a breathing accent line
with corner brackets), handles, labels and the hint. The highlight rectangle
eases towards its target at every 16 ms tick, and only the changed strips are
invalidated, so even 4K screens animate smoothly.

## The editor

Annotations are objects drawn in image coordinates; the canvas uses
`SetOrigin`/`SetScale` so everything (pen sizes, fonts, bitmaps) follows the
zoom. Undo/redo stores deep copies of the annotation list (the base bitmap is
shared until a crop replaces it). Blur annotations cache a filtered copy of
the pixels under them; the cache is dropped when they move or the base
changes. Export draws the base and the annotations into an offscreen
`B_RGBA32` bitmap.
