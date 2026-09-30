// Message constants, the application signature and the settings file layout
// shared by the airShot application and its input_server filter.
#pragma once
#include <SupportDefs.h>

namespace airshot {

constexpr const char* kAppSignature = "application/x-vnd.airOS-airShot";
constexpr const char* kAppName = "airShot";
constexpr const char* kSettingsDirectory = "airShot";
constexpr const char* kSettingsFile = "settings";

// Sent by the input_server filter (and by the Deskbar replicant, the launcher
// window and the command line) to start a capture.
constexpr uint32 kMsgCaptureFull = 'CapF';
constexpr uint32 kMsgCaptureWindow = 'CapW';
constexpr uint32 kMsgCaptureRegion = 'CapR';

// Application-internal messages.
constexpr uint32 kMsgShowMainWindow = 'ShMW';
constexpr uint32 kMsgShowSettings = 'ShSt';
constexpr uint32 kMsgSettingsChanged = 'StCh';
constexpr uint32 kMsgOverlayDone = 'OvDn';
constexpr uint32 kMsgOverlayCancelled = 'OvCn';
constexpr uint32 kMsgOpenEditor = 'OpEd';
constexpr uint32 kMsgAbout = 'Abut';
// From the editor: new default colour/width/font size and its window frame.
constexpr uint32 kMsgEditorState = 'EdSt';
constexpr uint32 kMsgMainWindowFrame = 'MwFr';
constexpr uint32 kMsgQuitApp = 'QApp';

// Capture kinds, also stored in messages as "kind".
enum CaptureKind {
	kCaptureFull = 0,
	kCaptureWindow = 1,
	kCaptureRegion = 2,
};

// What happens once a screenshot exists.
enum AfterCapture {
	kAfterOpenEditor = 0,
	kAfterCopy = 1,
	kAfterSave = 2,
};

}  // namespace airshot
