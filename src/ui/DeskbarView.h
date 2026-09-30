// airShot's Deskbar tray icon. Deskbar loads the airShot executable as an
// add-on and instantiates this view inside its own team; the view talks to
// the application through its signature and launches it when needed.
#pragma once
#include <View.h>

class BBitmap;
class BMessageRunner;

namespace airshot {

class DeskbarView : public BView {
public:
	explicit					DeskbarView(BRect frame);
	explicit					DeskbarView(BMessage* archive);
	virtual						~DeskbarView();

	static	BArchivable*		Instantiate(BMessage* archive);
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;

	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MessageReceived(BMessage* message);

	static const char*			kName;

private:
			void				_Init();
			void				_ShowMenu(BPoint where);
			void				_Poll();

			BBitmap*			fIcon;
			BMessageRunner*		fPoller;
			bool				fAppRunning;
};

}  // namespace airshot
