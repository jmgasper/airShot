#include "ScreenCapture.h"

#include <Application.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Messenger.h>
#include <Region.h>
#include <Roster.h>
#include <Screen.h>
#include <View.h>
#include <WindowInfo.h>
#include <WindowPrivate.h>

#include <stdlib.h>
#include <string.h>

namespace airshot {

BRect WindowEntry::CaptureFrame(bool includeDecorations, BRect screenFrame) const
{
	BRect result = includeDecorations ? decoratedFrame : frame;
	return result & screenFrame;
}


BBitmap* ScreenCapture::GrabScreen(bool includeCursor)
{
	BBitmap* bitmap = NULL;
	BScreen screen;
	// app_server does not reliably leave the cursor out (Haiku tickets #2988
	// and #2997), so hide it ourselves when it must not show.
	bool hidden = false;
	if (!includeCursor && be_app != NULL && !be_app->IsCursorHidden()) {
		be_app->HideCursor();
		hidden = true;
	}
	status_t status = screen.GetBitmap(&bitmap, includeCursor);
	if (hidden)
		be_app->ShowCursor();
	if (status != B_OK) {
		delete bitmap;
		return NULL;
	}
	return bitmap;
}


void ScreenCapture::ListWindows(std::vector<WindowEntry>& windows, team_id exclude)
{
	windows.clear();
	BRect screenFrame = BScreen().Frame();
	int32* tokens = NULL;
	int32 count = 0;
	if (BPrivate::get_window_order(current_workspace(), &tokens, &count) != B_OK
		|| tokens == NULL)
		return;

	WindowEntry desktop;
	bool haveDesktop = false;
	for (int32 i = 0; i < count; i++) {
		client_window_info* info = get_window_info(tokens[i]);
		if (info == NULL)
			continue;
		bool visible = info->layer >= 2 && !info->is_mini && info->show_hide_level <= 0;
		if (visible && info->team != exclude && info->feel != kMenuWindowFeel) {
			WindowEntry entry;
			entry.team = info->team;
			entry.token = tokens[i];
			entry.name = info->name;
			entry.frame.Set(info->window_left, info->window_top, info->window_right,
				info->window_bottom);
			entry.tabHeight = info->tab_height;
			entry.borderSize = info->border_size;
			entry.decoratedFrame = entry.frame;
			entry.decoratedFrame.InsetBy(-entry.borderSize, -entry.borderSize);
			entry.decoratedFrame.top -= entry.tabHeight;
			entry.desktop = info->feel == kDesktopWindowFeel;
			if (entry.desktop) {
				entry.frame = screenFrame;
				entry.decoratedFrame = screenFrame;
				entry.name = "Desktop";
				if (!haveDesktop) {
					desktop = entry;
					haveDesktop = true;
				}
			} else if (entry.decoratedFrame.Intersects(screenFrame))
				windows.push_back(entry);
		}
		free(info);
	}
	free(tokens);
	if (haveDesktop)
		windows.push_back(desktop);
}


status_t ScreenCapture::TabFrameFor(const WindowEntry& entry, BRect& tabFrame)
{
	if (entry.desktop || entry.tabHeight <= 0)
		return B_ERROR;
	BMessenger messenger(NULL, entry.team);
	if (!messenger.IsValid())
		return B_ERROR;
	// Find the window's index in its application by comparing frames, then
	// ask for its tab.
	for (int32 index = 0; index < 64; index++) {
		BMessage request(B_GET_PROPERTY);
		request.AddSpecifier("Frame");
		request.AddSpecifier("Window", index);
		BMessage reply;
		if (messenger.SendMessage(&request, &reply, 200000, 200000) != B_OK)
			return B_ERROR;
		if (reply.what == B_MESSAGE_NOT_UNDERSTOOD || reply.what == B_ERROR)
			return B_ERROR;
		BRect frame;
		if (reply.FindRect("result", &frame) != B_OK)
			continue;
		if (fabs(frame.left - entry.frame.left) > 1 || fabs(frame.top - entry.frame.top) > 1
			|| fabs(frame.right - entry.frame.right) > 1
			|| fabs(frame.bottom - entry.frame.bottom) > 1)
			continue;
		request.MakeEmpty();
		request.what = B_GET_PROPERTY;
		request.AddSpecifier("TabFrame");
		request.AddSpecifier("Window", index);
		reply.MakeEmpty();
		if (messenger.SendMessage(&request, &reply, 200000, 200000) != B_OK)
			return B_ERROR;
		return reply.FindRect("result", &tabFrame);
	}
	return B_ERROR;
}


BBitmap* ScreenCapture::Crop(const BBitmap* source, BRect rect, bool transparent)
{
	rect = rect & source->Bounds();
	if (!rect.IsValid())
		return NULL;
	rect.left = floorf(rect.left);
	rect.top = floorf(rect.top);
	rect.right = floorf(rect.right);
	rect.bottom = floorf(rect.bottom);
	BBitmap* result = new BBitmap(rect.OffsetToCopy(B_ORIGIN),
		transparent ? B_RGBA32 : B_RGB32);
	if (result->InitCheck() != B_OK
		|| result->ImportBits(source, rect.LeftTop(), B_ORIGIN, rect.Size()) != B_OK) {
		delete result;
		return NULL;
	}
	if (transparent) {
		// Screens have no alpha channel; make the copy opaque first.
		uint8* bits = (uint8*)result->Bits();
		int32 length = result->BitsLength();
		for (int32 i = 3; i < length; i += 4)
			bits[i] = 255;
	}
	return result;
}


void ScreenCapture::ClearTabSpace(BBitmap* bitmap, BRect frame, BRect tabFrame, float tabHeight)
{
	if (bitmap->ColorSpace() != B_RGBA32 || !frame.Contains(tabFrame) || tabHeight <= 0)
		return;
	BRegion space(frame);
	BRect below(frame);
	below.top += tabHeight;
	space.Exclude(below);
	space.Exclude(tabFrame);
	space.OffsetBy(-frame.left, -frame.top);

	uint8* bits = (uint8*)bitmap->Bits();
	int32 rowBytes = bitmap->BytesPerRow();
	BRect bounds = bitmap->Bounds();
	for (int32 i = 0; i < space.CountRects(); i++) {
		BRect rect = space.RectAt(i) & bounds;
		if (!rect.IsValid())
			continue;
		for (int32 y = (int32)rect.top; y <= (int32)rect.bottom; y++) {
			uint32* row = (uint32*)(bits + y * rowBytes);
			for (int32 x = (int32)rect.left; x <= (int32)rect.right; x++)
				row[x] = 0;
		}
	}
}

}  // namespace airshot
