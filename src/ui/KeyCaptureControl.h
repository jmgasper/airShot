// A control that shows a keyboard shortcut and records a new one when
// clicked: the next key combination pressed becomes the shortcut.
#pragma once
#include <Control.h>
#include <Message.h>

#include "HotKey.h"

namespace airshot {

constexpr uint32 kMsgHotKeyRecording = 'HkRc';  // "recording" bool, to the window

class KeyCaptureControl : public BControl {
public:
								KeyCaptureControl(const char* name, const HotKey& hotKey,
									BMessage* message);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MakeFocus(bool focus);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

			const HotKey&		Key() const { return fHotKey; }
			void				SetKey(const HotKey& hotKey);
			bool				IsRecording() const { return fRecording; }
			void				StartRecording();
			void				StopRecording();

	// Consumes a key-down message while recording; returns true if handled.
			bool				HandleKeyMessage(const BMessage* message);

private:
			void				_Notify();

			HotKey				fHotKey;
			bool				fRecording;
};

}  // namespace airshot
