// The things a user draws on a screenshot. Every annotation is a small
// object that knows how to draw itself in image coordinates, where it is,
// and how to move; the canvas keeps them in a list so they stay editable
// until the image is exported.
#pragma once
#include <Bitmap.h>
#include <GraphicsDefs.h>
#include <Point.h>
#include <Rect.h>
#include <String.h>
#include <View.h>

#include <memory>
#include <vector>

namespace airshot {

enum AnnotationType {
	kAnnotationArrow = 0,
	kAnnotationLine,
	kAnnotationRectangle,
	kAnnotationEllipse,
	kAnnotationPen,
	kAnnotationHighlighter,
	kAnnotationText,
	kAnnotationCounter,
	kAnnotationBlur,
};

struct Style {
	rgb_color color;
	float strokeWidth;
	float fontSize;
	bool filled;
	bool pixelate;  // blur annotations: mosaic instead of a soft blur
};

class Annotation {
public:
								Annotation(AnnotationType type, const Style& style);
	virtual						~Annotation();

	virtual	Annotation*			Clone() const = 0;
			AnnotationType		Type() const { return fType; }

	// Drawing happens in image coordinates; the view is already scaled.
	// The base bitmap is what blur annotations sample.
	virtual	void				Draw(BView* view, const BBitmap* base) = 0;
	// Bounding box in image coordinates, stroke included.
	virtual	BRect				Bounds() const = 0;
	virtual	bool				HitTest(BPoint where) const;
	virtual	void				MoveBy(BPoint delta) = 0;

	// Creation by dragging from one point to another.
	virtual	void				SetEndPoints(BPoint start, BPoint end);
	// Creation by a freehand stroke.
	virtual	void				AddPoint(BPoint point);
	// Whether the shape is large enough to keep after creation.
	virtual	bool				IsUsable() const;

	// Handles for the selection tool: image coordinates.
	virtual	int32				CountHandles() const { return 0; }
	virtual	BPoint				HandleAt(int32 index) const { return BPoint(); }
	virtual	void				MoveHandleTo(int32 index, BPoint where) {}

			const Style&		GetStyle() const { return fStyle; }
	virtual	void				SetStyle(const Style& style);

	// Blur annotations cache pixels of the base bitmap; a new base drops it.
	virtual	void				InvalidateCache() {}

	static	Annotation*			Create(AnnotationType type, const Style& style);

protected:
			AnnotationType		fType;
			Style				fStyle;
};

typedef std::shared_ptr<Annotation> AnnotationRef;


class LineAnnotation : public Annotation {
public:
								LineAnnotation(AnnotationType type, const Style& style);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	BRect				Bounds() const;
	virtual	bool				HitTest(BPoint where) const;
	virtual	void				MoveBy(BPoint delta);
	virtual	void				SetEndPoints(BPoint start, BPoint end);
	virtual	bool				IsUsable() const;
	virtual	int32				CountHandles() const { return 2; }
	virtual	BPoint				HandleAt(int32 index) const;
	virtual	void				MoveHandleTo(int32 index, BPoint where);

private:
			float				_HeadSize() const;

			BPoint				fStart;
			BPoint				fEnd;
};


class RectAnnotation : public Annotation {
public:
								RectAnnotation(AnnotationType type, const Style& style);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	BRect				Bounds() const;
	virtual	bool				HitTest(BPoint where) const;
	virtual	void				MoveBy(BPoint delta);
	virtual	void				SetEndPoints(BPoint start, BPoint end);
	virtual	bool				IsUsable() const;
	virtual	int32				CountHandles() const { return 4; }
	virtual	BPoint				HandleAt(int32 index) const;
	virtual	void				MoveHandleTo(int32 index, BPoint where);

			BRect				Frame() const { return fFrame; }

protected:
			BRect				fFrame;
};


class PenAnnotation : public Annotation {
public:
								PenAnnotation(AnnotationType type, const Style& style);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	BRect				Bounds() const;
	virtual	bool				HitTest(BPoint where) const;
	virtual	void				MoveBy(BPoint delta);
	virtual	void				AddPoint(BPoint point);
	virtual	bool				IsUsable() const;

private:
			float				_Width() const;

			std::vector<BPoint>	fPoints;
};


class TextAnnotation : public Annotation {
public:
								TextAnnotation(const Style& style);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	BRect				Bounds() const;
	virtual	void				MoveBy(BPoint delta);
	virtual	bool				IsUsable() const;

			void				SetText(const char* text);
			const char*			Text() const { return fText.String(); }
			void				SetPosition(BPoint position) { fPosition = position; }
			BPoint				Position() const { return fPosition; }

	static	void				PrepareFont(BFont& font, float size);

private:
			void				_Lines(std::vector<BString>& lines) const;

			BPoint				fPosition;  // top-left of the text block
			BString				fText;
};


class CounterAnnotation : public Annotation {
public:
								CounterAnnotation(const Style& style, BPoint center, int32 number);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	BRect				Bounds() const;
	virtual	bool				HitTest(BPoint where) const;
	virtual	void				MoveBy(BPoint delta);

			int32				Number() const { return fNumber; }
			float				Radius() const;

private:
			BPoint				fCenter;
			int32				fNumber;
};


class BlurAnnotation : public RectAnnotation {
public:
								BlurAnnotation(const Style& style);
	virtual	Annotation*			Clone() const;
	virtual	void				Draw(BView* view, const BBitmap* base);
	virtual	void				MoveBy(BPoint delta);
	virtual	void				SetEndPoints(BPoint start, BPoint end);
	virtual	void				MoveHandleTo(int32 index, BPoint where);
	virtual	void				SetStyle(const Style& style);
	virtual	void				InvalidateCache();

private:
			void				_Rebuild(const BBitmap* base);

			std::shared_ptr<BBitmap> fCache;
			BRect				fCacheFrame;
			bool				fCachePixelated;
};

// Image filters used by the blur annotation, on B_RGB(A)32 bitmaps.
void BoxBlur(BBitmap* bitmap, int32 radius, int32 passes);
void Pixelate(BBitmap* bitmap, int32 blockSize);

}  // namespace airshot
