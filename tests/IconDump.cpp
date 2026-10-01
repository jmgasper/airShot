// Dumps the runtime-drawn toolbar icons as PNG files for inspection:
//   icon_dump <output directory>
#include <Application.h>
#include <BitmapStream.h>
#include <File.h>
#include <String.h>
#include <TranslatorRoster.h>

#include <stdio.h>

#include "editor/ToolIcons.h"

using namespace airshot;

static void Save(BBitmap* bitmap, const char* path)
{
	BFile file(path, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	BBitmapStream stream(bitmap);
	BTranslatorRoster::Default()->Translate(&stream, NULL, NULL, &file, B_PNG_FORMAT);
	BBitmap* detached;
	stream.DetachBitmap(&detached);
	delete detached;
}

int main(int argc, char** argv)
{
	BApplication app("application/x-vnd.airOS-airShot-icondump");
	const char* dir = argc > 1 ? argv[1] : ".";
	for (int32 i = 0; i < kToolCount; i++) {
		BString path;
		path.SetToFormat("%s/tool-%d.png", dir, (int)i);
		Save(MakeToolIcon((Tool)i, 48), path.String());
	}
	for (int32 i = 0; i < kIconCount; i++) {
		BString path;
		path.SetToFormat("%s/action-%d.png", dir, (int)i);
		Save(MakeActionIcon((ActionIcon)i, 48), path.String());
	}
	printf("done\n");
	return 0;
}
