// Native screen-readback regression probe. An optional millisecond limit
// makes unexpectedly slow captures fail on a known test workstation.
#include <Application.h>
#include <Screen.h>

#include <stdio.h>
#include <stdlib.h>

#include "capture/ScreenCapture.h"

int main(int argc, char** argv)
{
	char* end = NULL;
	long limit = argc == 2 ? strtol(argv[1], &end, 10) : 0;
	if (argc > 2 || (argc == 2 && (end == argv[1] || *end != '\0'
			|| limit <= 0 || limit > 60000))) {
		fprintf(stderr, "usage: capture_timing [max-milliseconds (1..60000)]\n");
		return 2;
	}
	BApplication app("application/x-vnd.airOS-airShot-capture-timing");
	BRect frame = BScreen().Frame();
	bool passed = true;
	for (int i = 0; i < 6; i++) {
		bool cursor = (i % 2) != 0;
		bigtime_t start = system_time();
		BBitmap* bitmap = airshot::ScreenCapture::GrabScreen(cursor);
		bigtime_t elapsed = system_time() - start;
		if (bitmap == NULL || bitmap->InitCheck() != B_OK
			|| bitmap->Bounds().Width() != frame.Width()
			|| bitmap->Bounds().Height() != frame.Height()) {
			fprintf(stderr, "capture %d: missing or incorrectly sized bitmap\n", i + 1);
			delete bitmap;
			return 1;
		}
		printf("capture %d: %.0fx%.0f, cursor=%d, %.1f ms\n", i + 1,
			frame.Width() + 1, frame.Height() + 1, cursor, elapsed / 1000.0);
		if (limit != 0 && elapsed > limit * 1000LL)
			passed = false;
		delete bitmap;
	}
	return passed ? 0 : 1;
}
