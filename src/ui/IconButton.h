// Native button layout and interaction with full alpha blending for icons.
#pragma once

#include <Button.h>

namespace airshot {

class IconButton : public BButton {
public:
	IconButton(const char* name, const char* label, BMessage* message,
		bool lockable = false);

	void Draw(BRect updateRect) override;
	void MouseDown(BPoint where) override;
	void MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage) override;

private:
	bool fLockable;
	bool fInside;
};

}  // namespace airshot
