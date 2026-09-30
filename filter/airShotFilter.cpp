// airShot's input_server filter: watches every key press for the configured
// shortcuts and tells the airShot application to capture the screen, a
// window or a region. It runs inside input_server, so it stays small, never
// blocks the input thread, and reloads the settings file when it changes.
#include <Entry.h>
#include <InterfaceDefs.h>
#include <Locker.h>
#include <Looper.h>
#include <Messenger.h>
#include <Path.h>
#include <PathMonitor.h>
#include <Roster.h>
#include <add-ons/input_server/InputServerFilter.h>

#include <stdio.h>
#include <syslog.h>

#include "HotKey.h"
#include "Messages.h"
#include "Settings.h"

using namespace airshot;

namespace {

constexpr uint32 kMsgReloadSettings = 'RlSt';
constexpr uint32 kMsgDispatch = 'Dsp!';

// Owns the settings and delivers capture requests from its own thread.
class HotKeyDispatcher : public BLooper {
public:
	HotKeyDispatcher()
		:
		BLooper("airShot hotkey dispatcher", B_NORMAL_PRIORITY),
		fLock("airShot hotkeys")
	{
		BPath path;
		if (Settings::GetPath(path, false) == B_OK) {
			BPath directory;
			path.GetParent(&directory);
			fWatchedPath = directory.Path();
			BPrivate::BPathMonitor::StartWatching(fWatchedPath.String(),
				B_WATCH_DIRECTORY | B_WATCH_STAT | B_WATCH_FILES_ONLY, BMessenger(this));
		}
		_Reload();
	}

	~HotKeyDispatcher() override
	{
		if (!fWatchedPath.IsEmpty())
			BPrivate::BPathMonitor::StopWatching(BMessenger(this));
	}

	// Called from the input thread: which capture does this key press ask for?
	// Returns -1 when none matches.
	int32 Match(int32 key, uint32 modifiers)
	{
		if (!fLock.Lock())
			return -1;
		int32 result = -1;
		if (fSettings.hotKeysPaused)
			;
		else if (fSettings.fullHotKey.Matches(key, modifiers))
			result = kCaptureFull;
		else if (fSettings.windowHotKey.Matches(key, modifiers))
			result = kCaptureWindow;
		else if (fSettings.regionHotKey.Matches(key, modifiers))
			result = kCaptureRegion;
		fLock.Unlock();
		return result;
	}

	void MessageReceived(BMessage* message) override
	{
		switch (message->what) {
			case B_PATH_MONITOR:
			case kMsgReloadSettings:
				_Reload();
				break;
			case kMsgDispatch:
				_Dispatch(message->GetInt32("kind", kCaptureFull));
				break;
			default:
				BLooper::MessageReceived(message);
		}
	}

private:
	void _Reload()
	{
		Settings settings;
		settings.Load();
			// missing file: the defaults apply
		if (fLock.Lock()) {
			fSettings = settings;
			fLock.Unlock();
		}
	}

	void _Dispatch(int32 kind)
	{
		bool launch;
		if (fLock.Lock()) {
			launch = fSettings.launchOnHotKey;
			fLock.Unlock();
		} else
			launch = true;

		uint32 what = kMsgCaptureFull;
		if (kind == kCaptureWindow)
			what = kMsgCaptureWindow;
		else if (kind == kCaptureRegion)
			what = kMsgCaptureRegion;
		BMessage request(what);
		request.AddInt32("kind", kind);
		request.AddBool("hotkey", true);

		BMessenger app(kAppSignature);
		if (app.IsValid()) {
			app.SendMessage(&request);
			return;
		}
		if (!launch)
			return;
		// Command line arguments reach the application before ReadyToRun(),
		// so it knows not to open its window first.
		const char* option = kind == kCaptureWindow ? "--window"
			: (kind == kCaptureRegion ? "--region" : "--full");
		char* argv[] = {const_cast<char*>(option), NULL};
		status_t status = be_roster->Launch(kAppSignature, 1, argv);
		if (status != B_OK && status != B_ALREADY_RUNNING) {
			syslog(LOG_WARNING, "airShot filter: launching %s failed: %s", kAppSignature,
				strerror(status));
		}
	}

	BLocker fLock;
	Settings fSettings;
	BString fWatchedPath;
};


class AirShotFilter : public BInputServerFilter {
public:
	AirShotFilter()
		:
		fDispatcher(new HotKeyDispatcher),
		fSwallowKeyUp(-1)
	{
		fDispatcher->Run();
	}

	~AirShotFilter() override
	{
		if (fDispatcher->Lock())
			fDispatcher->Quit();
	}

	status_t InitCheck() override
	{
		return B_OK;
	}

	filter_result Filter(BMessage* message, BList* outList) override
	{
		switch (message->what) {
			case B_KEY_DOWN:
			case B_UNMAPPED_KEY_DOWN:
			{
				int32 key;
				int32 modifiers;
				if (message->FindInt32("key", &key) != B_OK
					|| message->FindInt32("modifiers", &modifiers) != B_OK)
					return B_DISPATCH_MESSAGE;
				// Holding the key must not fire again and again.
				if (message->GetInt32("be:key_repeat", 0) > 1)
					return key == fSwallowKeyUp ? B_SKIP_MESSAGE : B_DISPATCH_MESSAGE;
				int32 kind = fDispatcher->Match(key, (uint32)modifiers);
				if (kind < 0)
					return B_DISPATCH_MESSAGE;
				BMessage dispatch(kMsgDispatch);
				dispatch.AddInt32("kind", kind);
				fDispatcher->PostMessage(&dispatch);
				fSwallowKeyUp = key;
				return B_SKIP_MESSAGE;
			}

			case B_KEY_UP:
			case B_UNMAPPED_KEY_UP:
			{
				// Applications never saw the key go down; hide its release too.
				int32 key;
				if (fSwallowKeyUp >= 0 && message->FindInt32("key", &key) == B_OK
					&& key == fSwallowKeyUp) {
					fSwallowKeyUp = -1;
					return B_SKIP_MESSAGE;
				}
				break;
			}
		}
		return B_DISPATCH_MESSAGE;
	}

private:
	HotKeyDispatcher* fDispatcher;
	int32 fSwallowKeyUp;
};

}  // namespace


extern "C" BInputServerFilter* instantiate_input_filter()
{
	return new AirShotFilter;
}
