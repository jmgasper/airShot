// The small launcher window: one button per capture mode, showing its
// shortcut, plus settings. Closing it keeps airShot running for shortcuts.
#pragma once
#include <Button.h>
#include <StringView.h>
#include <Window.h>

#include "Settings.h"

namespace airshot {

class MainWindow : public BWindow {
public:
								MainWindow(const Settings& settings);

	virtual	void				MessageReceived(BMessage* message);
	virtual	bool				QuitRequested();

			void				Update(const Settings& settings);

private:
			BButton*			_MakeButton(const char* name, const char* label, int32 icon,
									uint32 command);

			BStringView*		fFullKey;
			BStringView*		fWindowKey;
			BStringView*		fRegionKey;
};

}  // namespace airshot
