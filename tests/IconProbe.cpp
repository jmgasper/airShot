// Probes offscreen bitmap drawing on the running app_server: fills a
// rectangle with several colour spaces, drawing modes and waits, and counts
// pixels inside the rectangle that do not hold the fill colour.
#include <Application.h>
#include <Bitmap.h>
#include <View.h>
#include <stdio.h>
#include <string.h>

static int Probe(color_space space, drawing_mode mode, bool snoozeAfter, bool polygon, bool clear)
{
	BBitmap* bitmap = new BBitmap(BRect(0, 0, 47, 47), space, true);
	if (clear)
		memset(bitmap->Bits(), 0, bitmap->BitsLength());
	BView* view = new BView(bitmap->Bounds(), "v", B_FOLLOW_NONE, 0);
	bitmap->AddChild(view);
	bitmap->Lock();
	view->SetDrawingMode(mode);
	view->SetHighColor(10, 20, 30, 255);
	BRect r(8, 8, 39, 39);
	if (polygon) {
		BPoint p[4] = {r.LeftTop(), r.RightTop(), r.RightBottom(), r.LeftBottom()};
		view->FillPolygon(p, 4);
	} else
		view->FillRect(r);
	view->Sync();
	bitmap->Unlock();
	if (snoozeAfter)
		snooze(100000);
	int bad = 0;
	uint8* bits = (uint8*)bitmap->Bits();
	for (int y = 10; y <= 37; y++) {
		for (int x = 10; x <= 37; x++) {
			uint8* px = bits + y * bitmap->BytesPerRow() + x * 4;
			if (px[0] != 30 || px[1] != 20 || px[2] != 10)
				bad++;
		}
	}
	bitmap->RemoveChild(view);
	delete view;
	delete bitmap;
	return bad;
}

int main()
{
	BApplication app("application/x-vnd.airOS-airShot-probe");
	const char* spaces[] = {"RGBA32", "RGB32"};
	color_space cs[] = {B_RGBA32, B_RGB32};
	const char* modes[] = {"copy", "over", "alpha"};
	drawing_mode dm[] = {B_OP_COPY, B_OP_OVER, B_OP_ALPHA};
	for (int s = 0; s < 2; s++)
		for (int m = 0; m < 3; m++)
			for (int poly = 0; poly < 2; poly++)
				for (int sn = 0; sn < 2; sn++) {
					int total = 0;
					for (int i = 0; i < 5; i++)
						total += Probe(cs[s], dm[m], sn, poly, true);
					printf("%-7s %-6s %-8s %-6s bad=%d/3920\n", spaces[s], modes[m],
						poly ? "polygon" : "fillrect", sn ? "snooze" : "nosnooze", total);
				}
	return 0;
}
