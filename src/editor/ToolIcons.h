// Embedded vector toolbar icons, rasterized at the requested size and tinted
// with the panel text colour. No installed icon font is required.
#pragma once
#include <Bitmap.h>

namespace airshot {

enum Tool {
	kToolSelect = 0,
	kToolArrow,
	kToolLine,
	kToolRectangle,
	kToolEllipse,
	kToolPen,
	kToolHighlighter,
	kToolText,
	kToolCounter,
	kToolBlur,
	kToolCrop,
	kToolCount,
};

enum ActionIcon {
	kIconUndo = 0,
	kIconRedo,
	kIconCopy,
	kIconSave,
	kIconFullScreen,
	kIconWindow,
	kIconRegion,
	kIconSettings,
	kIconCount,
};

const char* ToolName(Tool tool);
char ToolShortcut(Tool tool);

BBitmap* MakeToolIcon(Tool tool, float size);
BBitmap* MakeActionIcon(ActionIcon icon, float size);

}  // namespace airshot
