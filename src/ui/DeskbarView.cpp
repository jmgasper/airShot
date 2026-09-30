#include "DeskbarView.h"

#include <AppFileInfo.h>
#include <Bitmap.h>
#include <Deskbar.h>
#include <File.h>
#include <MenuItem.h>
#include <MessageRunner.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>
#include <Window.h>
#include <image.h>

#include <math.h>

#include "Messages.h"

namespace airshot {

const char* DeskbarView::kName = "airShot";

namespace {

constexpr uint32 kMsgPoll = 'poll';

// The executable this code was loaded from (inside Deskbar's team).
bool OwnImagePath(BString& path)
{
	image_info info;
	int32 cookie = 0;
	addr_t self = (addr_t)&OwnImagePath;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		if (self >= (addr_t)info.text && self < (addr_t)info.text + info.text_size) {
			path = info.name;
			return true;
		}
	}
	return false;
}


// Sends to the running application, or starts it with the request.
void SendToApp(uint32 what)
{
	BMessage message(what);
	BMessenger app(kAppSignature);
	if (app.IsValid()) {
		app.SendMessage(&message);
		return;
	}
	const char* option = NULL;
	switch (what) {
		case kMsgCaptureFull: option = "--full"; break;
		case kMsgCaptureWindow: option = "--window"; break;
		case kMsgCaptureRegion: option = "--region"; break;
		case kMsgShowSettings: option = "--settings"; break;
		default: break;
	}
	if (option != NULL) {
		char* argv[] = {const_cast<char*>(option), NULL};
		be_roster->Launch(kAppSignature, 1, argv);
	} else
		be_roster->Launch(kAppSignature);
}

}  // namespace


DeskbarView::DeskbarView(BRect frame)
	:
	BView(frame, kName, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW),
	fIcon(NULL),
	fPoller(NULL),
	fAppRunning(false)
{
	_Init();
}


DeskbarView::DeskbarView(BMessage* archive)
	:
	BView(archive),
	fIcon(NULL),
	fPoller(NULL),
	fAppRunning(false)
{
	_Init();
}


DeskbarView::~DeskbarView()
{
	delete fIcon;
	delete fPoller;
}


void DeskbarView::_Init()
{
	BString path;
	if (!OwnImagePath(path))
		return;
	BFile file(path.String(), B_READ_ONLY);
	BAppFileInfo info(&file);
	float size = Bounds().Height() + 1;
	fIcon = new BBitmap(BRect(0, 0, size - 1, size - 1), B_RGBA32);
	if (info.InitCheck() != B_OK || info.GetIcon(fIcon, (icon_size)size) != B_OK) {
		delete fIcon;
		fIcon = NULL;
	}
}


BArchivable* DeskbarView::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "airshot::DeskbarView"))
		return NULL;
	return new DeskbarView(archive);
}


status_t DeskbarView::Archive(BMessage* archive, bool deep) const
{
	status_t status = BView::Archive(archive, deep);
	if (status == B_OK)
		status = archive->AddString("add_on", kAppSignature);
	if (status == B_OK)
		status = archive->AddString("class", "airshot::DeskbarView");
	return status;
}


void DeskbarView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AdoptParentColors();
	if (ViewUIColor() == B_NO_COLOR)
		SetViewColor(Parent() != NULL ? Parent()->ViewColor() : B_TRANSPARENT_COLOR);
	SetLowColor(ViewColor());
	BMessage poll(kMsgPoll);
	fPoller = new BMessageRunner(BMessenger(this), &poll, 3000000);
	_Poll();
}


void DeskbarView::DetachedFromWindow()
{
	delete fPoller;
	fPoller = NULL;
	BView::DetachedFromWindow();
}


void DeskbarView::Draw(BRect updateRect)
{
	if (fIcon == NULL)
		return;
	SetDrawingMode(B_OP_ALPHA);
	if (fAppRunning)
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	else {
		// Dimmed while airShot is not running.
		SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
		SetHighColor(0, 0, 0, 110);
	}
	DrawBitmap(fIcon, Bounds().LeftTop());
}


void DeskbarView::MouseDown(BPoint where)
{
	_ShowMenu(ConvertToScreen(where));
}


void DeskbarView::MessageReceived(BMessage* message)
{
	if (message->what == kMsgPoll) {
		_Poll();
		return;
	}
	BView::MessageReceived(message);
}


void DeskbarView::_Poll()
{
	bool running = BMessenger(kAppSignature).IsValid();
	if (running != fAppRunning) {
		fAppRunning = running;
		Invalidate();
	}
}


void DeskbarView::_ShowMenu(BPoint where)
{
	BPopUpMenu* menu = new BPopUpMenu("airShot", false, false);
	menu->SetFont(be_plain_font);
	menu->AddItem(new BMenuItem("Capture full screen", new BMessage(kMsgCaptureFull)));
	menu->AddItem(new BMenuItem("Capture window", new BMessage(kMsgCaptureWindow)));
	menu->AddItem(new BMenuItem("Capture region", new BMessage(kMsgCaptureRegion)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Open airShot", new BMessage(kMsgShowMainWindow)));
	menu->AddItem(new BMenuItem("Settings" B_UTF8_ELLIPSIS, new BMessage(kMsgShowSettings)));
	menu->AddSeparatorItem();
	BMenuItem* quit = new BMenuItem("Quit airShot", new BMessage(kMsgQuitApp));
	quit->SetEnabled(fAppRunning);
	menu->AddItem(quit);
	menu->AddItem(new BMenuItem("Remove from Deskbar", new BMessage(B_QUIT_REQUESTED)));

	BMenuItem* chosen = menu->Go(where, false, true);
	if (chosen != NULL) {
		uint32 what = chosen->Message()->what;
		if (what == B_QUIT_REQUESTED) {
			BMessenger app(kAppSignature);
			if (app.IsValid()) {
				BMessage message(kMsgSettingsChanged);
				message.AddBool("show_in_deskbar", false);
				app.SendMessage(&message);
			}
			BDeskbar().RemoveItem(kName);
		} else
			SendToApp(what);
	}
	delete menu;
}

}  // namespace airshot


extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth, float maxHeight)
{
	float size = floorf(maxHeight > 0 ? maxHeight : 16);
	return new airshot::DeskbarView(BRect(0, 0, size - 1, size - 1));
}
