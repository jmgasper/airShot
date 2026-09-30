// The editor's colour swatches.
#pragma once
#include <GraphicsDefs.h>

namespace airshot {

const rgb_color kPalette[] = {
	{232, 48, 42, 255},    // red
	{255, 140, 0, 255},    // orange
	{250, 210, 30, 255},   // yellow
	{40, 185, 80, 255},    // green
	{30, 130, 255, 255},   // blue
	{160, 80, 220, 255},   // purple
	{20, 20, 20, 255},     // black
	{255, 255, 255, 255},  // white
};
const int32 kPaletteSize = sizeof(kPalette) / sizeof(kPalette[0]);

const float kStrokeWidths[] = {2, 4, 6, 10};
const int32 kStrokeWidthCount = 4;
const float kFontSizes[] = {16, 20, 24, 32, 48, 64};
const int32 kFontSizeCount = 6;

}  // namespace airshot
