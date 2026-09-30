#include "Export.h"

#include <Application.h>
#include <BitmapStream.h>
#include <Clipboard.h>
#include <Entry.h>
#include <File.h>
#include <IconUtils.h>
#include <NodeInfo.h>
#include <Notification.h>
#include <Path.h>
#include <Roster.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <View.h>

#include <stdio.h>
#include <time.h>

#include "Messages.h"

namespace airshot {

BBitmap* Export::Flatten(const BBitmap* base, const std::vector<AnnotationRef>& items)
{
	BBitmap* result = new BBitmap(base->Bounds(), B_RGBA32, true);
	if (result->InitCheck() != B_OK) {
		delete result;
		return NULL;
	}
	BView* view = new BView(base->Bounds(), "flatten", B_FOLLOW_NONE, 0);
	result->AddChild(view);
	if (result->Lock()) {
		view->SetDrawingMode(B_OP_COPY);
		view->SetHighColor(0, 0, 0, 0);
		view->FillRect(view->Bounds());
		view->SetDrawingMode(base->ColorSpace() == B_RGBA32 ? B_OP_ALPHA : B_OP_COPY);
		view->DrawBitmap(base, B_ORIGIN);
		for (const AnnotationRef& item : items)
			item->Draw(view, base);
		view->Sync();
		result->Unlock();
	}
	result->RemoveChild(view);
	delete view;
	return result;
}


status_t Export::SavePNG(const BBitmap* bitmap, const char* path)
{
	BFile file(path, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;
	// BBitmapStream wants to own its bitmap; give it a copy.
	BBitmap* copy = new BBitmap(bitmap);
	BBitmapStream stream(copy);
	status = BTranslatorRoster::Default()->Translate(&stream, NULL, NULL, &file,
		B_PNG_FORMAT, B_TRANSLATOR_BITMAP);
	BBitmap* detached = NULL;
	stream.DetachBitmap(&detached);
	delete detached;
	if (status != B_OK)
		return status;
	BNodeInfo info(&file);
	if (info.InitCheck() == B_OK)
		info.SetType("image/png");
	return B_OK;
}


status_t Export::CopyToClipboard(const BBitmap* bitmap)
{
	if (!be_clipboard->Lock())
		return B_ERROR;
	status_t status = B_ERROR;
	be_clipboard->Clear();
	BMessage* data = be_clipboard->Data();
	if (data != NULL) {
		BMessage archive;
		if (bitmap->Archive(&archive) == B_OK) {
			data->AddMessage("image/bitmap", &archive);
			status = be_clipboard->Commit();
		}
	}
	be_clipboard->Unlock();
	return status;
}


BString Export::DefaultFileName(const char* prefix)
{
	time_t now = time(NULL);
	struct tm local;
	localtime_r(&now, &local);
	char stamp[64];
	strftime(stamp, sizeof(stamp), "%Y-%m-%d at %H.%M.%S", &local);
	BString name(prefix != NULL && prefix[0] != '\0' ? prefix : "airShot");
	name << " " << stamp << ".png";
	return name;
}


BString Export::UniquePath(const char* folder, const char* fileName)
{
	BString base(fileName);
	BString extension;
	int32 dot = base.FindLast('.');
	if (dot > 0) {
		base.CopyInto(extension, dot, base.Length() - dot);
		base.Truncate(dot);
	}
	for (int32 attempt = 1; attempt < 1000; attempt++) {
		BString candidate(folder);
		candidate << "/" << base;
		if (attempt > 1)
			candidate << " " << attempt;
		candidate << extension;
		if (!BEntry(candidate.String()).Exists())
			return candidate;
	}
	BString fallback(folder);
	fallback << "/" << fileName;
	return fallback;
}


void Export::Notify(const char* title, const char* content, const BBitmap* preview)
{
	BNotification notification(B_INFORMATION_NOTIFICATION);
	notification.SetGroup(kAppName);
	notification.SetTitle(title);
	notification.SetContent(content);
	notification.SetMessageID(kAppName);
	if (preview != NULL)
		notification.SetIcon(preview);
	else {
		// The application's own icon.
		app_info info;
		if (be_app != NULL && be_app->GetAppInfo(&info) == B_OK) {
			BBitmap icon(BRect(0, 0, 31, 31), B_RGBA32);
			BNodeInfo nodeInfo;
			BNode node(&info.ref);
			if (nodeInfo.SetTo(&node) == B_OK && nodeInfo.GetIcon(&icon, B_LARGE_ICON) == B_OK)
				notification.SetIcon(&icon);
		}
	}
	notification.Send(4000000);
}

}  // namespace airshot
