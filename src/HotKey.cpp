#include "HotKey.h"

#include <stdio.h>
#include <ctype.h>

namespace airshot {

HotKey::HotKey(int32_t key, uint32_t modifiers, const std::string& keyName)
	:
	key(key),
	modifiers(modifiers & kModifierMask),
	label(MakeLabel(modifiers, keyName))
{
}


bool HotKey::Matches(int32_t pressedKey, uint32_t pressedModifiers) const
{
	return IsSet() && pressedKey == key
		&& (pressedModifiers & kModifierMask) == modifiers;
}


bool HotKey::operator==(const HotKey& other) const
{
	return key == other.key && modifiers == other.modifiers;
}


std::string HotKey::ModifierLabel(uint32_t modifiers)
{
	std::string result;
	if (modifiers & kShiftKey)
		result += "Shift+";
	if (modifiers & kControlKey)
		result += "Ctrl+";
	if (modifiers & kCommandKey)
		result += "Alt+";
	if (modifiers & kOptionKey)
		result += "Opt+";
	return result;
}


std::string HotKey::MakeLabel(uint32_t modifiers, const std::string& keyName)
{
	return ModifierLabel(modifiers) + keyName;
}


bool IsModifierKeyCode(int32_t key)
{
	switch (key) {
		case kKeyLeftShift:
		case kKeyRightShift:
		case kKeyLeftControl:
		case kKeyRightControl:
		case kKeyLeftCommand:
		case kKeyRightCommand:
		case kKeyLeftOption:
		case kKeyRightOption:
		case kKeyCapsLock:
		case kKeyNumLock:
		case kKeyScrollLock:
		case kKeyMenu:
			return true;
	}
	return false;
}


std::string KeyName(int32_t key, int32_t rawChar)
{
	if (key >= kKeyF1 && key <= kKeyF12) {
		char buffer[8];
		snprintf(buffer, sizeof(buffer), "F%d", (int)(key - kKeyF1 + 1));
		return buffer;
	}
	switch (key) {
		case kKeyEscape: return "Esc";
		case kKeyPrintScreen: return "Print Screen";
		case kKeyScrollLock: return "Scroll Lock";
		case kKeyPause: return "Pause";
		case kKeyBackspace: return "Backspace";
		case kKeyInsert: return "Insert";
		case kKeyHome: return "Home";
		case kKeyPageUp: return "Page Up";
		case kKeyTab: return "Tab";
		case kKeyDelete: return "Delete";
		case kKeyEnd: return "End";
		case kKeyPageDown: return "Page Down";
		case kKeyReturn: return "Enter";
		case kKeyNumpadEnter: return "Numpad Enter";
		case kKeySpace: return "Space";
		case kKeyUp: return "Up";
		case kKeyDown: return "Down";
		case kKeyLeft: return "Left";
		case kKeyRight: return "Right";
		case kKeyMenu: return "Menu";
	}
	// The numeric keypad shares characters with the main block; tell them apart.
	if (key >= 0x23 && key <= 0x25) {
		const char* names[] = {"Numpad /", "Numpad *", "Numpad -"};
		return names[key - 0x23];
	}
	if (key == 0x3a) return "Numpad +";
	if ((key >= 0x37 && key <= 0x39) || (key >= 0x48 && key <= 0x4a)
		|| (key >= 0x58 && key <= 0x5a) || key == 0x64 || key == 0x65) {
		char buffer[16];
		snprintf(buffer, sizeof(buffer), "Numpad %c", isprint(rawChar) ? (char)rawChar : '?');
		return buffer;
	}
	if (rawChar > 32 && rawChar < 127) {
		char buffer[2] = {(char)toupper(rawChar), 0};
		return buffer;
	}
	char buffer[16];
	snprintf(buffer, sizeof(buffer), "Key 0x%02x", (unsigned)key);
	return buffer;
}

}  // namespace airshot
