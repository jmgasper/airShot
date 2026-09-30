#include "KeyCaptureControl.h"

#include <ControlLook.h>
#include <InterfaceDefs.h>
#include <LayoutUtils.h>
#include <Window.h>

#include <math.h>

namespace airshot {

KeyCaptureControl::KeyCaptureControl(const char* name, const HotKey& hotKey, BMessage* message)
	:
	BControl(name, "", message, B_WILL_DRAW | B_NAVIGABLE),
	fHotKey(hotKey),
	fRecording(false)
{
}


void KeyCaptureControl::AttachedToWindow()
{
	BControl::AttachedToWindow();
	AdoptParentColors();
}


BSize KeyCaptureControl::MinSize()
{
	font_height fh;
	be_plain_font->GetHeight(&fh);
	float height = ceilf(fh.ascent + fh.descent) + 12;
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
		BSize(be_plain_font->StringWidth("Shift+Ctrl+Alt+Print Screen") + 24, height));
}


BSize KeyCaptureControl::PreferredSize()
{
	return MinSize();
}


void KeyCaptureControl::Draw(BRect updateRect)
{
	BRect rect = Bounds();
	rgb_color base = ui_color(B_CONTROL_BACKGROUND_COLOR);
	uint32 flags = be_control_look->Flags(this);
	if (fRecording)
		flags |= BControlLook::B_ACTIVATED | BControlLook::B_FOCUSED;
	be_control_look->DrawButtonFrame(this, rect, updateRect, base, ViewColor(), flags);
	be_control_look->DrawButtonBackground(this, rect, updateRect, base, flags);

	BString text;
	rgb_color textColor = ui_color(B_CONTROL_TEXT_COLOR);
	if (fRecording) {
		text = "Press a key combination" B_UTF8_ELLIPSIS;
		textColor = ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	} else if (fHotKey.IsSet())
		text = fHotKey.label.c_str();
	else {
		text = "Not set";
		textColor = tint_color(textColor, B_DISABLED_LABEL_TINT);
	}
	SetHighColor(textColor);
	SetLowColor(base);
	SetDrawingMode(B_OP_OVER);
	font_height fh;
	be_plain_font->GetHeight(&fh);
	float width = be_plain_font->StringWidth(text.String());
	DrawString(text.String(), BPoint(rect.left + (rect.Width() - width) / 2,
		rect.top + (rect.Height() - (fh.ascent + fh.descent)) / 2 + fh.ascent));
}


void KeyCaptureControl::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;
	if (fRecording)
		StopRecording();
	else
		StartRecording();
}


void KeyCaptureControl::MakeFocus(bool focus)
{
	BControl::MakeFocus(focus);
	if (!focus && fRecording)
		StopRecording();
}


void KeyCaptureControl::SetKey(const HotKey& hotKey)
{
	fHotKey = hotKey;
	Invalidate();
}


void KeyCaptureControl::StartRecording()
{
	fRecording = true;
	MakeFocus(true);
	Invalidate();
	if (Window() != NULL) {
		BMessage message(kMsgHotKeyRecording);
		message.AddBool("recording", true);
		message.AddPointer("control", this);
		Window()->PostMessage(&message, Window());
	}
}


void KeyCaptureControl::StopRecording()
{
	if (!fRecording)
		return;
	fRecording = false;
	Invalidate();
	if (Window() != NULL) {
		BMessage message(kMsgHotKeyRecording);
		message.AddBool("recording", false);
		message.AddPointer("control", this);
		Window()->PostMessage(&message, Window());
	}
}


bool KeyCaptureControl::HandleKeyMessage(const BMessage* message)
{
	if (!fRecording)
		return false;
	int32 key = message->GetInt32("key", 0);
	uint32 modifiers = (uint32)message->GetInt32("modifiers", 0);
	int32 rawChar = message->GetInt32("raw_char", 0);
	if (key == 0 || IsModifierKeyCode(key))
		return true;  // wait for the real key
	if (key == kKeyEscape && (modifiers & kModifierMask) == 0) {
		StopRecording();
		return true;
	}
	if ((key == kKeyBackspace || key == kKeyDelete) && (modifiers & kModifierMask) == 0) {
		fHotKey = HotKey();
		StopRecording();
		_Notify();
		return true;
	}
	fHotKey = HotKey(key, modifiers, KeyName(key, rawChar));
	StopRecording();
	_Notify();
	return true;
}


void KeyCaptureControl::KeyDown(const char* bytes, int32 numBytes)
{
	if (fRecording && Window() != NULL && HandleKeyMessage(Window()->CurrentMessage()))
		return;
	if (numBytes == 1 && (bytes[0] == B_SPACE || bytes[0] == B_ENTER)) {
		StartRecording();
		return;
	}
	BControl::KeyDown(bytes, numBytes);
}


void KeyCaptureControl::_Notify()
{
	Invoke();
}

}  // namespace airshot
