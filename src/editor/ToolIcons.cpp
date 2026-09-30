#include "ToolIcons.h"

#include <Font.h>
#include <InterfaceDefs.h>
#include <Shape.h>
#include <View.h>

#include <math.h>

namespace airshot {

namespace {

struct IconCanvas {
	BBitmap* bitmap;
	BView* view;
	float size;

	IconCanvas(float size)
		:
		size(size)
	{
		bitmap = new BBitmap(BRect(0, 0, size - 1, size - 1), B_RGBA32, true);
		view = new BView(bitmap->Bounds(), "icon", B_FOLLOW_NONE, 0);
		bitmap->AddChild(view);
		bitmap->Lock();
		view->SetDrawingMode(B_OP_COPY);
		view->SetHighColor(0, 0, 0, 0);
		view->FillRect(view->Bounds());
		view->SetDrawingMode(B_OP_ALPHA);
		view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		view->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
		view->SetHighColor(ui_color(B_PANEL_TEXT_COLOR));
		view->SetPenSize(fmaxf(1.5f, size / 11));
	}

	BBitmap* Finish()
	{
		view->Sync();
		bitmap->Unlock();
		bitmap->RemoveChild(view);
		delete view;
		return bitmap;
	}

	// Coordinates in a 0..1 unit square.
	BPoint P(float x, float y) const
	{
		return BPoint(x * (size - 1), y * (size - 1));
	}

	BRect R(float left, float top, float right, float bottom) const
	{
		return BRect(P(left, top), P(right, bottom));
	}

	void Line(float x0, float y0, float x1, float y1)
	{
		view->StrokeLine(P(x0, y0), P(x1, y1));
	}
};


void DrawArrowHead(IconCanvas& c, BPoint from, BPoint to, float length)
{
	float dx = to.x - from.x;
	float dy = to.y - from.y;
	float l = sqrtf(dx * dx + dy * dy);
	if (l < 0.01f)
		return;
	float ux = dx / l;
	float uy = dy / l;
	BPoint a(to.x - ux * length - uy * length * 0.55f, to.y - uy * length + ux * length * 0.55f);
	BPoint b(to.x - ux * length + uy * length * 0.55f, to.y - uy * length - ux * length * 0.55f);
	BPoint triangle[3] = {to, a, b};
	c.view->FillPolygon(triangle, 3);
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
	IconCanvas c(size);
	BView* v = c.view;
	rgb_color text = ui_color(B_PANEL_TEXT_COLOR);
	switch (tool) {
		case kToolSelect:
		{
			BPoint cursor[7] = {c.P(0.2f, 0.1f), c.P(0.2f, 0.78f), c.P(0.38f, 0.62f),
				c.P(0.5f, 0.9f), c.P(0.62f, 0.84f), c.P(0.5f, 0.58f), c.P(0.72f, 0.56f)};
			v->FillPolygon(cursor, 7);
			break;
		}
		case kToolArrow:
			c.Line(0.15f, 0.85f, 0.7f, 0.3f);
			DrawArrowHead(c, c.P(0.15f, 0.85f), c.P(0.86f, 0.14f), size * 0.32f);
			break;
		case kToolLine:
			c.Line(0.15f, 0.85f, 0.85f, 0.15f);
			break;
		case kToolRectangle:
			v->StrokeRect(c.R(0.14f, 0.2f, 0.86f, 0.8f));
			break;
		case kToolEllipse:
			v->StrokeEllipse(c.R(0.1f, 0.18f, 0.9f, 0.82f));
			break;
		case kToolPen:
		{
			BShape shape;
			shape.MoveTo(c.P(0.12f, 0.75f));
			shape.BezierTo(c.P(0.25f, 0.1f), c.P(0.45f, 1.0f), c.P(0.55f, 0.5f));
			shape.BezierTo(c.P(0.62f, 0.2f), c.P(0.75f, 0.35f), c.P(0.88f, 0.25f));
			v->StrokeShape(&shape);
			break;
		}
		case kToolHighlighter:
		{
			v->SetHighColor(250, 210, 30, 170);
			v->SetPenSize(size * 0.3f);
			v->SetLineMode(B_SQUARE_CAP, B_ROUND_JOIN);
			c.Line(0.2f, 0.5f, 0.8f, 0.5f);
			v->SetHighColor(text);
			v->SetPenSize(fmaxf(1.5f, size / 12));
			c.Line(0.15f, 0.36f, 0.85f, 0.36f);
			c.Line(0.15f, 0.64f, 0.7f, 0.64f);
			break;
		}
		case kToolText:
		{
			BFont font(be_bold_font);
			font.SetSize(size * 0.95f);
			v->SetFont(&font);
			font_height fh;
			font.GetHeight(&fh);
			float width = font.StringWidth("T");
			v->DrawString("T", BPoint((size - width) / 2, (size + fh.ascent - fh.descent) / 2));
			break;
		}
		case kToolCounter:
		{
			v->FillEllipse(c.R(0.08f, 0.08f, 0.92f, 0.92f));
			BFont font(be_bold_font);
			font.SetSize(size * 0.62f);
			v->SetFont(&font);
			font_height fh;
			font.GetHeight(&fh);
			float width = font.StringWidth("1");
			v->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			v->DrawString("1", BPoint((size - width) / 2, (size + fh.ascent - fh.descent) / 2 - 1));
			break;
		}
		case kToolBlur:
		{
			int shades[9] = {60, 130, 90, 160, 70, 120, 100, 150, 80};
			float cell = size / 3;
			for (int i = 0; i < 9; i++) {
				int x = i % 3;
				int y = i / 3;
				rgb_color color = text;
				color.alpha = shades[i] + 60;
				v->SetHighColor(color);
				v->FillRect(BRect(x * cell + 1, y * cell + 1, (x + 1) * cell - 1, (y + 1) * cell - 1));
			}
			break;
		}
		case kToolCrop:
			c.Line(0.3f, 0.05f, 0.3f, 0.7f);
			c.Line(0.3f, 0.7f, 0.95f, 0.7f);
			c.Line(0.05f, 0.3f, 0.7f, 0.3f);
			c.Line(0.7f, 0.3f, 0.7f, 0.95f);
			break;
		default:
			break;
	}
	return c.Finish();
}


BBitmap* MakeActionIcon(ActionIcon icon, float size)
{
	IconCanvas c(size);
	BView* v = c.view;
	rgb_color text = ui_color(B_PANEL_TEXT_COLOR);
	switch (icon) {
		case kIconUndo:
		case kIconRedo:
		{
			// A curved arrow; mirrored for redo.
			bool redo = icon == kIconRedo;
			float sx = redo ? -1 : 1;
			float cx = size / 2;
			BRect arc(cx - size * 0.32f, size * 0.22f, cx + size * 0.32f, size * 0.86f);
			v->StrokeArc(arc, redo ? 300 : 60, 180);
			BPoint tip = c.P(redo ? 0.82f : 0.18f, 0.42f);
			BPoint from = c.P(redo ? 0.68f : 0.32f, 0.2f);
			DrawArrowHead(c, from, tip, size * 0.3f);
			(void)sx;
			break;
		}
		case kIconCopy:
			v->StrokeRect(c.R(0.12f, 0.12f, 0.62f, 0.62f));
			v->FillRect(c.R(0.38f, 0.38f, 0.9f, 0.9f));
			break;
		case kIconSave:
			c.Line(0.5f, 0.08f, 0.5f, 0.62f);
			DrawArrowHead(c, c.P(0.5f, 0.2f), c.P(0.5f, 0.68f), size * 0.3f);
			c.Line(0.12f, 0.62f, 0.12f, 0.9f);
			c.Line(0.12f, 0.9f, 0.88f, 0.9f);
			c.Line(0.88f, 0.9f, 0.88f, 0.62f);
			break;
		case kIconFullScreen:
			v->StrokeRoundRect(c.R(0.06f, 0.14f, 0.94f, 0.8f), 2, 2);
			c.Line(0.35f, 0.92f, 0.65f, 0.92f);
			break;
		case kIconWindow:
		{
			v->StrokeRoundRect(c.R(0.08f, 0.22f, 0.92f, 0.9f), 2, 2);
			v->FillRect(c.R(0.08f, 0.08f, 0.55f, 0.22f));
			break;
		}
		case kIconRegion:
		{
			rgb_color dim = text;
			dim.alpha = 90;
			v->SetHighColor(dim);
			v->FillRect(c.R(0.04f, 0.04f, 0.96f, 0.96f));
			v->SetHighColor(text);
			float d = size * 0.22f;
			BRect r = c.R(0.28f, 0.28f, 0.8f, 0.8f);
			v->StrokeLine(BPoint(r.left, r.top + d), BPoint(r.left, r.top));
			v->StrokeLine(BPoint(r.left, r.top), BPoint(r.left + d, r.top));
			v->StrokeLine(BPoint(r.right - d, r.top), BPoint(r.right, r.top));
			v->StrokeLine(BPoint(r.right, r.top), BPoint(r.right, r.top + d));
			v->StrokeLine(BPoint(r.right, r.bottom - d), BPoint(r.right, r.bottom));
			v->StrokeLine(BPoint(r.right, r.bottom), BPoint(r.right - d, r.bottom));
			v->StrokeLine(BPoint(r.left + d, r.bottom), BPoint(r.left, r.bottom));
			v->StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.bottom - d));
			break;
		}
		case kIconSettings:
		{
			// A gear: a ring with teeth.
			float cx = size / 2;
			float cy = size / 2;
			float outer = size * 0.42f;
			float inner = size * 0.28f;
			for (int i = 0; i < 8; i++) {
				float angle = i * M_PI / 4;
				v->SetPenSize(size * 0.16f);
				v->SetLineMode(B_BUTT_CAP, B_MITER_JOIN);
				v->StrokeLine(BPoint(cx + cosf(angle) * inner, cy + sinf(angle) * inner),
					BPoint(cx + cosf(angle) * outer, cy + sinf(angle) * outer));
			}
			v->SetPenSize(size * 0.14f);
			v->StrokeEllipse(BPoint(cx, cy), inner, inner);
			break;
		}
		default:
			break;
	}
	return c.Finish();
}

}  // namespace airshot
