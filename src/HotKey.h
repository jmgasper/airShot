// A keyboard shortcut: a raw key code plus modifier keys. Shared by the
// application and the input_server filter, and free of Haiku headers so the
// unit tests can build it on any platform.
#pragma once
#include <stdint.h>
#include <string>

namespace airshot {

// The modifier bits that identify a shortcut; the values are Haiku's
// (B_SHIFT_KEY, B_COMMAND_KEY, B_CONTROL_KEY, B_OPTION_KEY). Lock keys and the
// left/right variants are ignored when matching.
enum {
	kShiftKey = 0x01,
	kCommandKey = 0x02,
	kControlKey = 0x04,
	kOptionKey = 0x40,
	kModifierMask = kShiftKey | kCommandKey | kControlKey | kOptionKey,
};

// Raw key codes used for the defaults and for naming keys (Haiku's layout
// independent codes, see InterfaceDefs.h).
enum {
	kKeyEscape = 0x01,
	kKeyF1 = 0x02,
	kKeyF12 = 0x0d,
	kKeyPrintScreen = 0x0e,
	kKeyScrollLock = 0x0f,
	kKeyPause = 0x10,
	kKeyBackspace = 0x1e,
	kKeyInsert = 0x1f,
	kKeyHome = 0x20,
	kKeyPageUp = 0x21,
	kKeyNumLock = 0x22,
	kKeyTab = 0x26,
	kKeyDelete = 0x34,
	kKeyEnd = 0x35,
	kKeyPageDown = 0x36,
	kKeyCapsLock = 0x3b,
	kKeyReturn = 0x47,
	kKeyLeftShift = 0x4b,
	kKeyRightShift = 0x56,
	kKeyUp = 0x57,
	kKeyLeftControl = 0x5c,
	kKeyLeftCommand = 0x5d,
	kKeySpace = 0x5e,
	kKeyRightCommand = 0x5f,
	kKeyRightControl = 0x60,
	kKeyLeft = 0x61,
	kKeyDown = 0x62,
	kKeyRight = 0x63,
	kKeyNumpadEnter = 0x5b,
	kKeyLeftOption = 0x66,
	kKeyRightOption = 0x67,
	kKeyMenu = 0x68,
};

struct HotKey {
	int32_t key = 0;         // raw key code; 0 means "not set"
	uint32_t modifiers = 0;  // masked with kModifierMask
	std::string label;       // human readable, e.g. "Ctrl+Print Screen"

	HotKey() = default;
	HotKey(int32_t key, uint32_t modifiers, const std::string& keyName);

	bool IsSet() const { return key != 0; }
	bool Matches(int32_t pressedKey, uint32_t pressedModifiers) const;
	bool operator==(const HotKey& other) const;
	bool operator!=(const HotKey& other) const { return !(*this == other); }

	// "Shift+Ctrl+Alt+Opt+<keyName>" in Haiku's modifier order.
	static std::string MakeLabel(uint32_t modifiers, const std::string& keyName);
	static std::string ModifierLabel(uint32_t modifiers);
};

// True for keys that only make sense as modifiers and cannot be a shortcut's
// main key.
bool IsModifierKeyCode(int32_t key);

// A display name for a key press, from the raw key code and the character the
// key produces without modifiers ("raw_char" in a B_KEY_DOWN message).
std::string KeyName(int32_t key, int32_t rawChar);

}  // namespace airshot
