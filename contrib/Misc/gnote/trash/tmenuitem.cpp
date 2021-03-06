/*
 Copyright (c) 2005 hetappi
 All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to
 deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to
 do so, provided that the above copyright notice(s) and this permission
 notice appear in all copies of the Software and that both the above
 copyright notice(s) and this permission notice appear in supporting
 documentation.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
 LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR
 ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Except as contained in this notice, the name of a copyright holder shall not
 be used in advertising or otherwise to promote the sale, use or other
 dealings in this Software without prior written authorization of the
 copyright holder.
*/
#include "trash/tmenuitem.hpp"

namespace trash {
	//
	TMenuItem::TMenuItem(Type type, const char* title) : type(type), listener(0) {
		this->title = String(title);
	}
	//
	TMenuItem::TMenuItem(const char* title) : type(TYPE_ITEM), listener(0) {
		this->title = String(title);
	}
	//
	void TMenuItem::paint(Graphics* g) {
		paintAt(g, 0, 0);
	}
	//
	void TMenuItem::paintAt(Graphics* g, int x, int y) {
		switch (type) {
			case TYPE_ITEM:
				if (getEnabled()) {
					if (getFocused()) {
						g->setColor(Color::blue);
						g->drawRect(x, y, getWidth(), getHeight());
						g->setColor(COLOR_BLUEGRAY);
						g->fillRect(x + 1, y + 1, getWidth() - 2, getHeight() - 2);
					} else {
						g->setColor(getBackground());
						g->fillRect(x, y, getWidth(), getHeight());
					}
					g->setColor(getForeground());
					g->drawString(title, x + 10, y + 3);
				} else {
					g->setColor(getBackground());
					g->fillRect(x, y, getWidth(), getHeight());
					g->setColor(Color::gray);
					g->drawString(title, 10 + x, 3 + y);
				}
				break;
			case TYPE_SEPARATOR:
				g->setColor(getForeground());
				g->drawLine(x + 5, y + 3, x + getWidth() - 5, y + 3);
				break;
			default:
				break;
		}
	}
	//
	void TMenuItem::processEvent(Event* event) {
		if (listener) {
			listener->processEvent(event);
		}
	}
}
