// Dumps the native vector toolbar icons as PNG files for inspection:
//   icon_dump <output directory> [size]
#include <Application.h>
#include <BitmapStream.h>
#include <File.h>
#include <String.h>
#include <TranslatorRoster.h>

#include <stdio.h>
#include <stdlib.h>

#include "editor/ToolIcons.h"

using namespace airshot;

static bool Save(BBitmap* bitmap, const char* path)
{
	if (bitmap == NULL || bitmap->InitCheck() != B_OK) {
		fprintf(stderr, "Failed to render %s\n", path);
		delete bitmap;
		return false;
	}
	// Catch missing/blank artwork or an accidental opaque background.
	int32 transparent = 0, visible = 0;
	for (int32 y = 0; y <= bitmap->Bounds().IntegerHeight(); y++) {
		const uint8* pixel = (const uint8*)bitmap->Bits() + y * bitmap->BytesPerRow();
		for (int32 x = 0; x <= bitmap->Bounds().IntegerWidth(); x++, pixel += 4) {
			if (pixel[3] == 0)
				transparent++;
			else
				visible++;
		}
	}
	if (transparent == 0 || visible == 0) {
		fprintf(stderr, "Invalid icon coverage: %s\n", path);
		delete bitmap;
		return false;
	}
	BFile file(path, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	BBitmapStream stream(bitmap);
	status_t status = file.InitCheck();
	if (status == B_OK)
		status = BTranslatorRoster::Default()->Translate(&stream, NULL, NULL, &file, B_PNG_FORMAT);
	BBitmap* detached = NULL;
	stream.DetachBitmap(&detached);
	delete detached;
	if (status != B_OK)
		fprintf(stderr, "Failed to export %s: %ld\n", path, (long)status);
	return status == B_OK;
}

int main(int argc, char** argv)
{
	BApplication app("application/x-vnd.airOS-airShot-icondump");
	const char* dir = argc > 1 ? argv[1] : ".";
	float size = argc > 2 ? atof(argv[2]) : 48;
	bool okay = true;
	for (int32 i = 0; i < kToolCount; i++) {
		BString path;
		path.SetToFormat("%s/tool-%d.png", dir, (int)i);
		okay = Save(MakeToolIcon((Tool)i, size), path.String()) && okay;
	}
	for (int32 i = 0; i < kIconCount; i++) {
		BString path;
		path.SetToFormat("%s/action-%d.png", dir, (int)i);
		okay = Save(MakeActionIcon((ActionIcon)i, size), path.String()) && okay;
	}
	printf("%s: %d icons at %.0f px\n", okay ? "passed" : "FAILED", kToolCount + kIconCount, size);
	return okay ? 0 : 1;
}
