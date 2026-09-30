#include "Annotation.h"

#include <Font.h>
#include <Shape.h>

#include <math.h>
#include <string.h>

namespace airshot {

namespace {

float DistanceToSegment(BPoint p, BPoint a, BPoint b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float length2 = dx * dx + dy * dy;
	float t = 0;
	if (length2 > 0)
		t = fmaxf(0, fminf(1, ((p.x - a.x) * dx + (p.y - a.y) * dy) / length2));
	float x = a.x + t * dx - p.x;
	float y = a.y + t * dy - p.y;
	return sqrtf(x * x + y * y);
}


BRect NormalizeRect(BPoint a, BPoint b)
{
	return BRect(fminf(a.x, b.x), fminf(a.y, b.y), fmaxf(a.x, b.x), fmaxf(a.y, b.y));
}


void SetStrokeStyle(BView* view, const Style& style, uint8 alpha = 255)
{
	rgb_color color = style.color;
	color.alpha = alpha;
	view->SetHighColor(color);
	view->SetPenSize(style.strokeWidth);
	view->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);
	if (alpha < 255) {
		view->SetDrawingMode(B_OP_ALPHA);
		view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	} else
		view->SetDrawingMode(B_OP_OVER);
}

}  // namespace


//	#pragma mark - Annotation


Annotation::Annotation(AnnotationType type, const Style& style)
	:
	fType(type),
	fStyle(style)
{
}


Annotation::~Annotation()
{
}


bool Annotation::HitTest(BPoint where) const
{
	return Bounds().Contains(where);
}


void Annotation::SetEndPoints(BPoint start, BPoint end)
{
}


void Annotation::AddPoint(BPoint point)
{
}


bool Annotation::IsUsable() const
{
	return true;
}


void Annotation::SetStyle(const Style& style)
{
	fStyle = style;
}


Annotation* Annotation::Create(AnnotationType type, const Style& style)
{
	switch (type) {
		case kAnnotationArrow:
		case kAnnotationLine:
			return new LineAnnotation(type, style);
		case kAnnotationRectangle:
		case kAnnotationEllipse:
			return new RectAnnotation(type, style);
		case kAnnotationPen:
		case kAnnotationHighlighter:
			return new PenAnnotation(type, style);
		case kAnnotationText:
			return new TextAnnotation(style);
		case kAnnotationCounter:
			return new CounterAnnotation(style, BPoint(0, 0), 1);
		case kAnnotationBlur:
			return new BlurAnnotation(style);
	}
	return NULL;
}


//	#pragma mark - LineAnnotation


LineAnnotation::LineAnnotation(AnnotationType type, const Style& style)
	:
	Annotation(type, style),
	fStart(0, 0),
	fEnd(0, 0)
{
}


Annotation* LineAnnotation::Clone() const
{
	return new LineAnnotation(*this);
}


float LineAnnotation::_HeadSize() const
{
	return fmaxf(10, fStyle.strokeWidth * 3.5f);
}


void LineAnnotation::Draw(BView* view, const BBitmap* base)
{
	SetStrokeStyle(view, fStyle);
	float dx = fEnd.x - fStart.x;
	float dy = fEnd.y - fStart.y;
	float length = sqrtf(dx * dx + dy * dy);
	if (fType == kAnnotationArrow && length > 1) {
		float head = _HeadSize();
		float ux = dx / length;
		float uy = dy / length;
		// Stop the shaft short of the tip so the head's point stays sharp.
		BPoint shaftEnd(fEnd.x - ux * head * 0.8f, fEnd.y - uy * head * 0.8f);
		view->StrokeLine(fStart, shaftEnd);
		BPoint head1(fEnd.x - ux * head - uy * head * 0.5f, fEnd.y - uy * head + ux * head * 0.5f);
		BPoint head2(fEnd.x - ux * head + uy * head * 0.5f, fEnd.y - uy * head - ux * head * 0.5f);
		BPoint triangle[3] = {fEnd, head1, head2};
		view->FillPolygon(triangle, 3);
		view->SetPenSize(1);
		view->StrokePolygon(triangle, 3, true);
	} else
		view->StrokeLine(fStart, fEnd);
	view->SetPenSize(1);
}


BRect LineAnnotation::Bounds() const
{
	float margin = fStyle.strokeWidth + (fType == kAnnotationArrow ? _HeadSize() : 2);
	return NormalizeRect(fStart, fEnd).InsetByCopy(-margin, -margin);
}


bool LineAnnotation::HitTest(BPoint where) const
{
	return DistanceToSegment(where, fStart, fEnd) <= fStyle.strokeWidth / 2 + 5;
}


void LineAnnotation::MoveBy(BPoint delta)
{
	fStart += delta;
	fEnd += delta;
}


void LineAnnotation::SetEndPoints(BPoint start, BPoint end)
{
	fStart = start;
	fEnd = end;
}


bool LineAnnotation::IsUsable() const
{
	return fabsf(fEnd.x - fStart.x) >= 3 || fabsf(fEnd.y - fStart.y) >= 3;
}


BPoint LineAnnotation::HandleAt(int32 index) const
{
	return index == 0 ? fStart : fEnd;
}


void LineAnnotation::MoveHandleTo(int32 index, BPoint where)
{
	if (index == 0)
		fStart = where;
	else
		fEnd = where;
}


//	#pragma mark - RectAnnotation


RectAnnotation::RectAnnotation(AnnotationType type, const Style& style)
	:
	Annotation(type, style),
	fFrame(0, 0, -1, -1)
{
}


Annotation* RectAnnotation::Clone() const
{
	return new RectAnnotation(*this);
}


void RectAnnotation::Draw(BView* view, const BBitmap* base)
{
	if (fStyle.filled) {
		SetStrokeStyle(view, fStyle, 90);
		if (fType == kAnnotationEllipse)
			view->FillEllipse(fFrame);
		else
			view->FillRect(fFrame);
	}
	SetStrokeStyle(view, fStyle);
	// Stroke centred on the frame so the shape covers exactly what was dragged.
	BRect frame = fFrame.InsetByCopy(fStyle.strokeWidth / 2, fStyle.strokeWidth / 2);
	if (fType == kAnnotationEllipse)
		view->StrokeEllipse(frame);
	else
		view->StrokeRoundRect(frame, 2, 2);
	view->SetPenSize(1);
}


BRect RectAnnotation::Bounds() const
{
	return fFrame.InsetByCopy(-fStyle.strokeWidth - 2, -fStyle.strokeWidth - 2);
}


bool RectAnnotation::HitTest(BPoint where) const
{
	if (!Bounds().Contains(where))
		return false;
	if (fStyle.filled || fType == kAnnotationBlur)
		return true;
	// Only the outline is a target, so things behind an empty box stay reachable.
	BRect inner = fFrame.InsetByCopy(fStyle.strokeWidth + 5, fStyle.strokeWidth + 5);
	return !inner.IsValid() || !inner.Contains(where);
}


void RectAnnotation::MoveBy(BPoint delta)
{
	fFrame.OffsetBy(delta);
}


void RectAnnotation::SetEndPoints(BPoint start, BPoint end)
{
	fFrame = NormalizeRect(start, end);
}


bool RectAnnotation::IsUsable() const
{
	return fFrame.Width() >= 3 && fFrame.Height() >= 3;
}


BPoint RectAnnotation::HandleAt(int32 index) const
{
	switch (index) {
		case 0: return fFrame.LeftTop();
		case 1: return fFrame.RightTop();
		case 2: return fFrame.RightBottom();
		default: return fFrame.LeftBottom();
	}
}


void RectAnnotation::MoveHandleTo(int32 index, BPoint where)
{
	BPoint opposite = HandleAt((index + 2) % 4);
	fFrame = NormalizeRect(opposite, where);
}


//	#pragma mark - PenAnnotation


PenAnnotation::PenAnnotation(AnnotationType type, const Style& style)
	:
	Annotation(type, style)
{
}


Annotation* PenAnnotation::Clone() const
{
	return new PenAnnotation(*this);
}


float PenAnnotation::_Width() const
{
	return fType == kAnnotationHighlighter ? fStyle.strokeWidth * 3.5f : fStyle.strokeWidth;
}


void PenAnnotation::Draw(BView* view, const BBitmap* base)
{
	if (fPoints.empty())
		return;
	Style style = fStyle;
	style.strokeWidth = _Width();
	SetStrokeStyle(view, style, fType == kAnnotationHighlighter ? 110 : 255);
	if (fType == kAnnotationHighlighter)
		view->SetLineMode(B_SQUARE_CAP, B_ROUND_JOIN);
	if (fPoints.size() == 1)
		view->StrokeLine(fPoints[0], fPoints[0]);
	else
		view->StrokePolygon(&fPoints[0], fPoints.size(), false);
	view->SetPenSize(1);
}


BRect PenAnnotation::Bounds() const
{
	if (fPoints.empty())
		return BRect(0, 0, -1, -1);
	BRect bounds(fPoints[0], fPoints[0]);
	for (const BPoint& point : fPoints) {
		bounds.left = fminf(bounds.left, point.x);
		bounds.top = fminf(bounds.top, point.y);
		bounds.right = fmaxf(bounds.right, point.x);
		bounds.bottom = fmaxf(bounds.bottom, point.y);
	}
	return bounds.InsetByCopy(-_Width() - 2, -_Width() - 2);
}


bool PenAnnotation::HitTest(BPoint where) const
{
	if (!Bounds().Contains(where))
		return false;
	float tolerance = _Width() / 2 + 5;
	if (fPoints.size() == 1)
		return DistanceToSegment(where, fPoints[0], fPoints[0]) <= tolerance;
	for (size_t i = 1; i < fPoints.size(); i++) {
		if (DistanceToSegment(where, fPoints[i - 1], fPoints[i]) <= tolerance)
			return true;
	}
	return false;
}


void PenAnnotation::MoveBy(BPoint delta)
{
	for (BPoint& point : fPoints)
		point += delta;
}


void PenAnnotation::AddPoint(BPoint point)
{
	if (!fPoints.empty()) {
		BPoint last = fPoints.back();
		if (fabsf(last.x - point.x) < 1.5f && fabsf(last.y - point.y) < 1.5f)
			return;
	}
	fPoints.push_back(point);
}


bool PenAnnotation::IsUsable() const
{
	return !fPoints.empty();
}


//	#pragma mark - TextAnnotation


TextAnnotation::TextAnnotation(const Style& style)
	:
	Annotation(kAnnotationText, style),
	fPosition(0, 0)
{
}


Annotation* TextAnnotation::Clone() const
{
	return new TextAnnotation(*this);
}


void TextAnnotation::PrepareFont(BFont& font, float size)
{
	font = *be_bold_font;
	font.SetSize(size);
}


void TextAnnotation::_Lines(std::vector<BString>& lines) const
{
	int32 start = 0;
	while (start <= fText.Length()) {
		int32 end = fText.FindFirst('\n', start);
		if (end < 0)
			end = fText.Length();
		BString line;
		fText.CopyInto(line, start, end - start);
		lines.push_back(line);
		start = end + 1;
	}
}


void TextAnnotation::Draw(BView* view, const BBitmap* base)
{
	BFont font;
	PrepareFont(font, fStyle.fontSize);
	view->SetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	float lineHeight = ceilf(fh.ascent + fh.descent + fh.leading);
	std::vector<BString> lines;
	_Lines(lines);
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	float y = fPosition.y + fh.ascent;
	// A soft dark shadow keeps light text readable on light screenshots and
	// vice versa.
	rgb_color shadow = {0, 0, 0, 110};
	if (fStyle.color.red + fStyle.color.green + fStyle.color.blue < 200)
		shadow = (rgb_color){255, 255, 255, 120};
	float offset = fmaxf(1, fStyle.fontSize / 18);
	for (const BString& line : lines) {
		view->SetHighColor(shadow);
		view->DrawString(line.String(), BPoint(fPosition.x + offset, y + offset));
		view->SetHighColor(fStyle.color);
		view->DrawString(line.String(), BPoint(fPosition.x, y));
		y += lineHeight;
	}
}


BRect TextAnnotation::Bounds() const
{
	BFont font;
	PrepareFont(font, fStyle.fontSize);
	font_height fh;
	font.GetHeight(&fh);
	float lineHeight = ceilf(fh.ascent + fh.descent + fh.leading);
	std::vector<BString> lines;
	_Lines(lines);
	float width = 0;
	for (const BString& line : lines)
		width = fmaxf(width, font.StringWidth(line.String()));
	BRect bounds(fPosition.x, fPosition.y, fPosition.x + width,
		fPosition.y + lineHeight * lines.size());
	return bounds.InsetByCopy(-4, -4);
}


void TextAnnotation::MoveBy(BPoint delta)
{
	fPosition += delta;
}


bool TextAnnotation::IsUsable() const
{
	BString trimmed(fText);
	trimmed.Trim();
	return trimmed.Length() > 0;
}


void TextAnnotation::SetText(const char* text)
{
	fText = text;
	// Trailing newlines only add empty space.
	while (fText.Length() > 0 && fText[fText.Length() - 1] == '\n')
		fText.Truncate(fText.Length() - 1);
}


//	#pragma mark - CounterAnnotation


CounterAnnotation::CounterAnnotation(const Style& style, BPoint center, int32 number)
	:
	Annotation(kAnnotationCounter, style),
	fCenter(center),
	fNumber(number)
{
}


Annotation* CounterAnnotation::Clone() const
{
	return new CounterAnnotation(*this);
}


float CounterAnnotation::Radius() const
{
	return fmaxf(12, fStyle.fontSize * 0.7f);
}


void CounterAnnotation::Draw(BView* view, const BBitmap* base)
{
	float radius = Radius();
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	view->SetHighColor(0, 0, 0, 70);
	view->FillEllipse(BPoint(fCenter.x + 1, fCenter.y + 2), radius, radius);
	view->SetHighColor(fStyle.color);
	view->FillEllipse(fCenter, radius, radius);
	view->SetPenSize(2);
	view->SetHighColor(255, 255, 255, 255);
	view->StrokeEllipse(fCenter, radius - 1, radius - 1);
	view->SetPenSize(1);

	BFont font(be_bold_font);
	font.SetSize(radius * 1.15f);
	view->SetFont(&font);
	BString text;
	text << fNumber;
	font_height fh;
	font.GetHeight(&fh);
	float width = font.StringWidth(text.String());
	// Dark numbers on light colours.
	bool light = fStyle.color.red + fStyle.color.green + fStyle.color.blue > 500;
	view->SetHighColor(light ? make_color(20, 20, 20) : make_color(255, 255, 255));
	view->DrawString(text.String(), BPoint(fCenter.x - width / 2,
		fCenter.y + (fh.ascent - fh.descent) / 2));
}


BRect CounterAnnotation::Bounds() const
{
	float radius = Radius() + 3;
	return BRect(fCenter.x - radius, fCenter.y - radius, fCenter.x + radius, fCenter.y + radius);
}


bool CounterAnnotation::HitTest(BPoint where) const
{
	float dx = where.x - fCenter.x;
	float dy = where.y - fCenter.y;
	return sqrtf(dx * dx + dy * dy) <= Radius() + 3;
}


void CounterAnnotation::MoveBy(BPoint delta)
{
	fCenter += delta;
}


//	#pragma mark - BlurAnnotation


BlurAnnotation::BlurAnnotation(const Style& style)
	:
	RectAnnotation(kAnnotationBlur, style),
	fCacheFrame(0, 0, -1, -1),
	fCachePixelated(false)
{
}


Annotation* BlurAnnotation::Clone() const
{
	return new BlurAnnotation(*this);
}


void BlurAnnotation::_Rebuild(const BBitmap* base)
{
	fCache.reset();
	BRect source = fFrame & base->Bounds();
	source.left = floorf(source.left);
	source.top = floorf(source.top);
	source.right = floorf(source.right);
	source.bottom = floorf(source.bottom);
	if (!source.IsValid())
		return;
	BBitmap* cache = new BBitmap(source.OffsetToCopy(B_ORIGIN), B_RGBA32);
	if (cache->InitCheck() != B_OK
		|| cache->ImportBits(base, source.LeftTop(), B_ORIGIN, source.Size()) != B_OK) {
		delete cache;
		return;
	}
	if (fStyle.pixelate) {
		int32 block = (int32)fmaxf(6, fminf(source.Width(), source.Height()) / 12);
		Pixelate(cache, fminf(block, 24));
	} else {
		int32 radius = (int32)fmaxf(4, fminf(source.Width(), source.Height()) / 24);
		BoxBlur(cache, fminf(radius, 14), 3);
	}
	fCache.reset(cache);
	fCacheFrame = source;
	fCachePixelated = fStyle.pixelate;
}


void BlurAnnotation::Draw(BView* view, const BBitmap* base)
{
	if (base == NULL)
		return;
	if (fCache.get() == NULL || fCacheFrame != (fFrame & base->Bounds())
		|| fCachePixelated != fStyle.pixelate)
		_Rebuild(base);
	if (fCache.get() == NULL)
		return;
	view->SetDrawingMode(B_OP_COPY);
	view->DrawBitmap(fCache.get(), fCache->Bounds(), fCacheFrame);
}


void BlurAnnotation::MoveBy(BPoint delta)
{
	RectAnnotation::MoveBy(delta);
	InvalidateCache();
}


void BlurAnnotation::SetEndPoints(BPoint start, BPoint end)
{
	RectAnnotation::SetEndPoints(start, end);
	fFrame.left = floorf(fFrame.left);
	fFrame.top = floorf(fFrame.top);
	fFrame.right = floorf(fFrame.right);
	fFrame.bottom = floorf(fFrame.bottom);
	InvalidateCache();
}


void BlurAnnotation::MoveHandleTo(int32 index, BPoint where)
{
	RectAnnotation::MoveHandleTo(index, where);
	InvalidateCache();
}


void BlurAnnotation::SetStyle(const Style& style)
{
	RectAnnotation::SetStyle(style);
	InvalidateCache();
}


void BlurAnnotation::InvalidateCache()
{
	fCache.reset();
	fCacheFrame = BRect(0, 0, -1, -1);
}


//	#pragma mark - filters


void BoxBlur(BBitmap* bitmap, int32 radius, int32 passes)
{
	int32 width = bitmap->Bounds().IntegerWidth() + 1;
	int32 height = bitmap->Bounds().IntegerHeight() + 1;
	int32 rowBytes = bitmap->BytesPerRow();
	uint8* bits = (uint8*)bitmap->Bits();
	if (width < 2 || height < 2 || radius < 1)
		return;
	std::vector<uint8> temporary(rowBytes * height);
	int32 window = radius * 2 + 1;

	for (int32 pass = 0; pass < passes; pass++) {
		// Horizontal: a running sum per channel.
		for (int32 y = 0; y < height; y++) {
			uint8* row = bits + y * rowBytes;
			uint8* out = &temporary[y * rowBytes];
			int32 sum[4] = {0, 0, 0, 0};
			for (int32 x = -radius; x <= radius; x++) {
				int32 sx = x < 0 ? 0 : (x >= width ? width - 1 : x);
				for (int32 c = 0; c < 4; c++)
					sum[c] += row[sx * 4 + c];
			}
			for (int32 x = 0; x < width; x++) {
				for (int32 c = 0; c < 4; c++)
					out[x * 4 + c] = sum[c] / window;
				int32 leave = x - radius;
				int32 enter = x + radius + 1;
				leave = leave < 0 ? 0 : leave;
				enter = enter >= width ? width - 1 : enter;
				for (int32 c = 0; c < 4; c++)
					sum[c] += row[enter * 4 + c] - row[leave * 4 + c];
			}
		}
		// Vertical, from the temporary buffer back into the bitmap.
		for (int32 x = 0; x < width; x++) {
			int32 sum[4] = {0, 0, 0, 0};
			for (int32 y = -radius; y <= radius; y++) {
				int32 sy = y < 0 ? 0 : (y >= height ? height - 1 : y);
				uint8* pixel = &temporary[sy * rowBytes + x * 4];
				for (int32 c = 0; c < 4; c++)
					sum[c] += pixel[c];
			}
			for (int32 y = 0; y < height; y++) {
				uint8* out = bits + y * rowBytes + x * 4;
				for (int32 c = 0; c < 4; c++)
					out[c] = sum[c] / window;
				int32 leave = y - radius;
				int32 enter = y + radius + 1;
				leave = leave < 0 ? 0 : leave;
				enter = enter >= height ? height - 1 : enter;
				uint8* enterPixel = &temporary[enter * rowBytes + x * 4];
				uint8* leavePixel = &temporary[leave * rowBytes + x * 4];
				for (int32 c = 0; c < 4; c++)
					sum[c] += enterPixel[c] - leavePixel[c];
			}
		}
	}
}


void Pixelate(BBitmap* bitmap, int32 blockSize)
{
	int32 width = bitmap->Bounds().IntegerWidth() + 1;
	int32 height = bitmap->Bounds().IntegerHeight() + 1;
	int32 rowBytes = bitmap->BytesPerRow();
	uint8* bits = (uint8*)bitmap->Bits();
	if (blockSize < 2)
		return;
	for (int32 by = 0; by < height; by += blockSize) {
		for (int32 bx = 0; bx < width; bx += blockSize) {
			int32 endX = bx + blockSize > width ? width : bx + blockSize;
			int32 endY = by + blockSize > height ? height : by + blockSize;
			int32 sum[4] = {0, 0, 0, 0};
			int32 count = 0;
			for (int32 y = by; y < endY; y++) {
				for (int32 x = bx; x < endX; x++) {
					uint8* pixel = bits + y * rowBytes + x * 4;
					for (int32 c = 0; c < 4; c++)
						sum[c] += pixel[c];
					count++;
				}
			}
			for (int32 y = by; y < endY; y++) {
				for (int32 x = bx; x < endX; x++) {
					uint8* pixel = bits + y * rowBytes + x * 4;
					for (int32 c = 0; c < 4; c++)
						pixel[c] = sum[c] / count;
				}
			}
		}
	}
}

}  // namespace airshot
