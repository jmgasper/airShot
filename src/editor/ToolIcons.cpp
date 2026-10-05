#include "ToolIcons.h"

#include <IconUtils.h>
#include <InterfaceDefs.h>

#include <math.h>
#include <string.h>

#include "ToolIconData.h"

namespace airshot {

namespace {

static_assert(sizeof(kToolIcons) / sizeof(kToolIcons[0]) == kToolCount,
	"one vector icon per tool");
static_assert(sizeof(kActionIcons) / sizeof(kActionIcons[0]) == kIconCount,
	"one vector icon per action");

BBitmap* RenderIcon(const VectorIcon& icon, float size)
{
	if (!isfinite(size) || size < 1 || size > 1024)
		return NULL;
	int32 pixels = (int32)ceilf(size);
	BBitmap* bitmap = new BBitmap(BRect(0, 0, pixels - 1, pixels - 1), B_RGBA32);
	if (bitmap->InitCheck() != B_OK) {
		delete bitmap;
		return NULL;
	}
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
	// The native vector rasterizer supplies coverage directly, avoiding the
	// app_server's offscreen drawing/alpha quirks and any font dependency.
	if (BIconUtils::GetVectorIcon(icon.data, icon.size, bitmap) != B_OK) {
		delete bitmap;
		return NULL;
	}

	// All artwork is a monochrome mask. Preserve its antialiased coverage
	// and transparent cut-outs while matching the current panel text colour.
	rgb_color color = ui_color(B_PANEL_TEXT_COLOR);
	for (int32 y = 0; y < pixels; y++) {
		uint8* pixel = (uint8*)bitmap->Bits() + y * bitmap->BytesPerRow();
		for (int32 x = 0; x < pixels; x++, pixel += 4) {
			pixel[0] = color.blue;
			pixel[1] = color.green;
			pixel[2] = color.red;
		}
	}
	return bitmap;
}

}  // namespace


const char* ToolName(Tool tool)
{
	switch (tool) {
		case kToolSelect: return "Select";
		case kToolArrow: return "Arrow";
		case kToolLine: return "Line";
		case kToolRectangle: return "Rectangle";
		case kToolEllipse: return "Ellipse";
		case kToolPen: return "Pen";
		case kToolHighlighter: return "Highlighter";
		case kToolText: return "Text";
		case kToolCounter: return "Counter";
		case kToolBlur: return "Blur";
		case kToolCrop: return "Crop";
		default: return "";
	}
}


char ToolShortcut(Tool tool)
{
	switch (tool) {
		case kToolSelect: return 'V';
		case kToolArrow: return 'A';
		case kToolLine: return 'L';
		case kToolRectangle: return 'R';
		case kToolEllipse: return 'E';
		case kToolPen: return 'P';
		case kToolHighlighter: return 'H';
		case kToolText: return 'T';
		case kToolCounter: return 'N';
		case kToolBlur: return 'B';
		case kToolCrop: return 'C';
		default: return 0;
	}
}


BBitmap* MakeToolIcon(Tool tool, float size)
{
	if (tool < 0 || tool >= kToolCount)
		return NULL;
	return RenderIcon(kToolIcons[tool], size);
}


BBitmap* MakeActionIcon(ActionIcon icon, float size)
{
	if (icon < 0 || icon >= kIconCount)
		return NULL;
	return RenderIcon(kActionIcons[icon], size);
}

}  // namespace airshot
