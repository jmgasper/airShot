// Turning a screenshot plus its annotations into pixels, files and clipboard
// contents.
#pragma once
#include <Bitmap.h>
#include <String.h>

#include <vector>

#include "Annotation.h"

namespace airshot {

class Export {
public:
	// A new B_RGBA32 bitmap with the annotations painted onto the base.
	static	BBitmap*			Flatten(const BBitmap* base, const std::vector<AnnotationRef>& items);

	static	status_t			SavePNG(const BBitmap* bitmap, const char* path);
	static	status_t			CopyToClipboard(const BBitmap* bitmap);

	// "airShot 2026-09-30 at 14.03.22.png"
	static	BString				DefaultFileName(const char* prefix);
	// folder/fileName, with " 2", " 3", ... inserted when the name is taken.
	static	BString				UniquePath(const char* folder, const char* fileName);

	// A desktop notification with airShot's icon.
	static	void				Notify(const char* title, const char* content,
									const BBitmap* preview = NULL);
};

}  // namespace airshot
