#include "IconButton.h"

#include <Bitmap.h>
#include <ControlLook.h>
#include <String.h>

#include <math.h>

namespace airshot {

IconButton::IconButton(const char* name, const char* label, BMessage* message, bool lockable)
	:
	BButton(name, label, message),
	fLockable(lockable),
	fInside(false)
{
}


void IconButton::Draw(BRect updateRect)
{
	BRect rect(Bounds());
	rgb_color base = ui_color(B_CONTROL_BACKGROUND_COLOR);
	rgb_color text = ui_color(B_CONTROL_TEXT_COLOR);
	uint32 flags = be_control_look->Flags(this);
	if (IsDefault())
		flags |= BControlLook::B_DEFAULT_BUTTON;
	if (IsFlat() && !IsTracking())
		flags |= BControlLook::B_FLAT;
	if (fInside)
		flags |= BControlLook::B_HOVER;

	PushState();
	be_control_look->DrawButtonFrame(this, rect, updateRect, base, ViewColor(), flags);
	be_control_look->DrawButtonBackground(this, rect, updateRect, base, flags);

	const BBitmap* icon = IconBitmap((Value() == B_CONTROL_OFF
		? B_INACTIVE_ICON_BITMAP : B_ACTIVE_ICON_BITMAP)
		| (IsEnabled() ? 0 : B_DISABLED_ICON_BITMAP));
	BString label(Label() != NULL ? Label() : "");
	float iconWidth = icon != NULL ? icon->Bounds().Width() + 1 : 0;
	float spacing = icon != NULL && !label.IsEmpty() ? be_control_look->DefaultLabelSpacing() : 0;
	BFont font;
	GetFont(&font);
	font.TruncateString(&label, B_TRUNCATE_END, fmaxf(0, rect.Width() + 1 - iconWidth - spacing));
	float labelWidth = ceilf(font.StringWidth(label.String()));
	float left = floorf(rect.left + (rect.Width() + 1 - iconWidth - spacing - labelWidth) / 2);
	if (icon != NULL) {
		BPoint position(left, floorf(rect.top + (rect.Height() - icon->Bounds().Height()) / 2));
		// BControlLook::DrawLabel uses B_OP_OVER, which treats all nonzero
		// alpha as opaque. B_OP_ALPHA preserves vector coverage and the
		// disabled bitmap's reduced opacity over the native button surface.
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		DrawBitmap(icon, position);
	}
	if (!label.IsEmpty()) {
		BRect labelRect(left + iconWidth + spacing, rect.top,
			left + iconWidth + spacing + labelWidth - 1, rect.bottom);
		SetDrawingMode(B_OP_COPY);
		be_control_look->DrawLabel(this, label.String(), labelRect, updateRect, base, flags,
			BAlignment(B_ALIGN_CENTER, B_ALIGN_MIDDLE), &text);
	}
	PopState();
}


void IconButton::MouseDown(BPoint where)
{
	if (fLockable) {
		SetBehavior((modifiers() & B_SHIFT_KEY) != 0 || Value() == B_CONTROL_ON
			? B_TOGGLE_BEHAVIOR : B_BUTTON_BEHAVIOR);
		Message()->SetInt32("behavior", Behavior());
	}
	BButton::MouseDown(where);
}


void IconButton::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	fInside = transit == B_ENTERED_VIEW || transit == B_INSIDE_VIEW;
	BButton::MouseMoved(where, transit, dragMessage);
}

}  // namespace airshot
