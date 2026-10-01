// Second probe: does the first drawing call on a fresh offscreen view
// misrender, and does a warm-up call or a Sync() before it help?
#include <Application.h>
#include <Bitmap.h>
#include <View.h>
#include <stdio.h>
#include <string.h>

enum Warmup { kNone, kSync, kDummyLine, kDummyPoint, kClearTransparent, kClearZero };

static int StrokeProbe(Warmup warmup, bool alpha)
{
	BBitmap* bitmap = new BBitmap(BRect(0, 0, 47, 47), B_RGBA32, true);
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
	BView* view = new BView(bitmap->Bounds(), "v", B_FOLLOW_NONE, 0);
	bitmap->AddChild(view);
	bitmap->Lock();
	view->SetDrawingMode(alpha ? B_OP_ALPHA : B_OP_OVER);
	if (alpha)
		view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view->SetHighColor(10, 20, 30, 255);
	view->SetPenSize(4);
	view->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
	if (warmup == kClearTransparent || warmup == kClearZero) {
		drawing_mode mode = view->DrawingMode();
		view->SetDrawingMode(B_OP_COPY);
		if (warmup == kClearTransparent)
			view->SetHighColor(B_TRANSPARENT_COLOR);
		else
			view->SetHighColor(0, 0, 0, 0);
		view->FillRect(view->Bounds());
		view->SetDrawingMode(mode);
		view->SetHighColor(10, 20, 30, 255);
	}
	if (warmup == kSync)
		view->Sync();
	else if (warmup == kDummyLine) {
		view->SetHighColor(0, 0, 0, 0);
		view->StrokeLine(BPoint(0, 0), BPoint(0, 0));
		view->SetHighColor(10, 20, 30, 255);
	} else if (warmup == kDummyPoint) {
		view->SetHighColor(0, 0, 0, 0);
		view->FillRect(BRect(0, 0, 0, 0));
		view->SetHighColor(10, 20, 30, 255);
	}
	BRect r(8, 8, 39, 39);
	BPoint points[5] = {r.LeftTop(), r.RightTop(), r.RightBottom(), r.LeftBottom(), r.LeftTop()};
	view->StrokePolygon(points, 5, false);
	view->Sync();
	bitmap->Unlock();
	// The interior (well inside the 4 px stroke) must stay transparent.
	int bad = 0;
	uint8* bits = (uint8*)bitmap->Bits();
	for (int y = 14; y <= 33; y++)
		for (int x = 14; x <= 33; x++)
			if (bits[y * bitmap->BytesPerRow() + x * 4 + 3] != 0)
				bad++;
	bitmap->RemoveChild(view);
	delete view;
	delete bitmap;
	return bad;
}

static int TranslucentProbe(Warmup warmup)
{
	BBitmap* bitmap = new BBitmap(BRect(0, 0, 47, 47), B_RGBA32, true);
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
	BView* view = new BView(bitmap->Bounds(), "v", B_FOLLOW_NONE, 0);
	bitmap->AddChild(view);
	bitmap->Lock();
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view->SetHighColor(10, 20, 30, 120);
	if (warmup == kClearTransparent || warmup == kClearZero) {
		drawing_mode mode = view->DrawingMode();
		view->SetDrawingMode(B_OP_COPY);
		if (warmup == kClearTransparent)
			view->SetHighColor(B_TRANSPARENT_COLOR);
		else
			view->SetHighColor(0, 0, 0, 0);
		view->FillRect(view->Bounds());
		view->SetDrawingMode(mode);
		view->SetHighColor(10, 20, 30, 120);
	}
	if (warmup == kSync)
		view->Sync();
	else if (warmup == kDummyLine) {
		view->StrokeLine(BPoint(0, 0), BPoint(0, 0));
	}
	BRect r(8, 8, 39, 39);
	BPoint points[4] = {r.LeftTop(), r.RightTop(), r.RightBottom(), r.LeftBottom()};
	view->FillPolygon(points, 4);
	view->Sync();
	bitmap->Unlock();
	// Every interior pixel should carry the same value.
	uint8* bits = (uint8*)bitmap->Bits();
	uint32 first = *(uint32*)(bits + 20 * bitmap->BytesPerRow() + 20 * 4);
	int bad = 0;
	for (int y = 10; y <= 37; y++)
		for (int x = 10; x <= 37; x++)
			if (*(uint32*)(bits + y * bitmap->BytesPerRow() + x * 4) != first)
				bad++;
	printf("  (pixel %08x)", (unsigned)first);
	bitmap->RemoveChild(view);
	delete view;
	delete bitmap;
	return bad;
}

int main()
{
	BApplication app("application/x-vnd.airOS-airShot-probe2");
	const char* names[] = {"none", "sync", "dummyline", "dummypoint", "cleartransp", "clearzero"};
	for (int w = 0; w < 6; w++) {
		printf("stroke over  warmup=%-10s bad=%d/400\n", names[w], StrokeProbe((Warmup)w, false));
		printf("stroke alpha warmup=%-10s bad=%d/400\n", names[w], StrokeProbe((Warmup)w, true));
	}
	for (int w = 0; w < 6; w++) {
		int bad = TranslucentProbe((Warmup)w);
		printf(" translucent warmup=%-10s bad=%d/784\n", names[w], bad);
	}
	return 0;
}
