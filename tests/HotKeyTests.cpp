// Tests for the shortcut model shared by the application and the filter.
#include "HotKey.h"

#include <stdio.h>
#include <string.h>

using namespace airshot;

static int failures = 0;

#define CHECK(condition) \
	do { \
		if (!(condition)) { \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
			failures++; \
		} \
	} while (0)


int main()
{
	// Matching ignores lock keys and the left/right modifier variants.
	HotKey printScreen(kKeyPrintScreen, 0, "Print Screen");
	CHECK(printScreen.IsSet());
	CHECK(printScreen.Matches(kKeyPrintScreen, 0));
	CHECK(printScreen.Matches(kKeyPrintScreen, 0x08 /* caps lock */ | 0x20 /* num lock */));
	CHECK(!printScreen.Matches(kKeyPrintScreen, kShiftKey));
	CHECK(!printScreen.Matches(kKeyF1, 0));

	HotKey ctrlPrint(kKeyPrintScreen, kControlKey, "Print Screen");
	CHECK(ctrlPrint.Matches(kKeyPrintScreen, kControlKey));
	CHECK(ctrlPrint.Matches(kKeyPrintScreen, kControlKey | 0x100 /* left control */));
	CHECK(!ctrlPrint.Matches(kKeyPrintScreen, kControlKey | kShiftKey));
	CHECK(ctrlPrint.label == "Ctrl+Print Screen");

	// Labels follow Haiku's order: Shift, Ctrl, Alt (command), Opt (option).
	HotKey all(0x12, kShiftKey | kControlKey | kCommandKey | kOptionKey, "1");
	CHECK(all.label == "Shift+Ctrl+Alt+Opt+1");
	CHECK(all.modifiers == (kShiftKey | kControlKey | kCommandKey | kOptionKey));

	// Unset shortcuts never match.
	HotKey none;
	CHECK(!none.IsSet());
	CHECK(!none.Matches(0, 0));

	// Equality is about the keys, not the label.
	HotKey a(kKeyF1, kCommandKey, "F1");
	HotKey b(kKeyF1, kCommandKey, "F-1");
	CHECK(a == b);
	CHECK(a != printScreen);

	// Key names.
	CHECK(KeyName(kKeyF1, 0x10) == "F1");
	CHECK(KeyName(kKeyF12, 0x10) == "F12");
	CHECK(KeyName(kKeyPrintScreen, 0x10) == "Print Screen");
	CHECK(KeyName(0x12, '1') == "1");
	CHECK(KeyName(0x40, 'a') == "A");
	CHECK(KeyName(kKeySpace, ' ') == "Space");
	CHECK(KeyName(kKeyLeft, 0x1c) == "Left");
	CHECK(KeyName(0x37, '7') == "Numpad 7");
	CHECK(KeyName(0x7f, 0) == "Key 0x7f");

	// Modifier keys alone are not shortcuts.
	CHECK(IsModifierKeyCode(kKeyLeftShift));
	CHECK(IsModifierKeyCode(kKeyRightOption));
	CHECK(IsModifierKeyCode(kKeyCapsLock));
	CHECK(!IsModifierKeyCode(kKeyPrintScreen));
	CHECK(!IsModifierKeyCode(0x40));

	if (failures == 0)
		printf("HotKey tests: all passed\n");
	else
		printf("HotKey tests: %d failure(s)\n", failures);
	return failures == 0 ? 0 : 1;
}
