// This file is in the public domain.
// There are no restrictions on any sort of usage of this file.

#ifndef __SYSTEM_MONA_FORMS_FORM_H__
#define __SYSTEM_MONA_FORMS_FORM_H__

#include <gui/System/Mona/Forms/Control.h>

namespace System { namespace Mona { namespace Forms
{
	class Form : public Control
	{
		friend class Control;
	
	protected:
		bool isCloseButtonPushed;
		enum NCState { NCState_None, NCState_TitleBar, NCState_CloseButton } ncState;
	
	private:
		double opacity;
#ifdef MONA
		dword overlap;
#else
		System::Drawing::Point ptRevRect;
#endif
		_P<System::Drawing::Bitmap> formBuffer;
	
	public:
		virtual String get_TypeName() { return "System.Mona.Forms.Form"; }
		Form();
		virtual ~Form();
		
		virtual void Hide();
		virtual void Create();
		virtual void Dispose();
		
		bool CheckPoint(int x, int y);
		inline double get_Opacity() { return this->opacity; }
		void set_Opacity(double op);
	
	protected:
		virtual void Erase();
		virtual NCState NCHitTest(int x, int y);
#ifndef MONA
		void DrawReversibleRectangle();
#endif
		virtual void OnPaint();
		virtual void OnNCMouseMove(_P<MouseEventArgs> e);
		virtual void OnNCMouseDown(_P<MouseEventArgs> e);
		virtual void OnNCMouseUp(_P<MouseEventArgs> e);
	};
}}}

#endif  // __SYSTEM_MONA_FORMS_FORM_H__
