// Grabbing the screen, listing the windows on it and cutting pieces out of
// the grabbed bitmap.
#pragma once
#include <Bitmap.h>
#include <Rect.h>
#include <String.h>
#include <OS.h>

#include <vector>

namespace airshot {

struct WindowEntry {
	team_id team;
	int32 token;
	BString name;
	BRect frame;           // the window's content area, screen coordinates
	BRect decoratedFrame;  // content plus border and tab row
	float tabHeight;
	float borderSize;
	bool desktop;          // the desktop window: covers the whole screen

	// The rectangle to capture for this window.
	BRect CaptureFrame(bool includeDecorations, BRect screenFrame) const;
};

class ScreenCapture {
public:
	// The whole screen, in B_RGB32; the caller owns the result.
	static	BBitmap*			GrabScreen(bool includeCursor);

	// The visible windows of the current workspace, front to back, without
	// the windows of the excluded team (ours) and menus. The desktop comes
	// last so that empty desktop space still selects something.
	static	void				ListWindows(std::vector<WindowEntry>& windows, team_id exclude);

	// The tab rectangle of a window (screen coordinates), asked from the
	// owning application; fails when the application does not answer.
	static	status_t			TabFrameFor(const WindowEntry& entry, BRect& tabFrame);

	// A copy of one rectangle of the source. The result is B_RGBA32 when
	// transparent is set, so callers can clear parts of it.
	static	BBitmap*			Crop(const BBitmap* source, BRect rect, bool transparent = false);

	// Makes the area beside a window's tab transparent, like Haiku's own
	// Screenshot does. The bitmap must be B_RGBA32 and cover frame.
	static	void				ClearTabSpace(BBitmap* bitmap, BRect frame, BRect tabFrame,
									float tabHeight);
};

}  // namespace airshot
