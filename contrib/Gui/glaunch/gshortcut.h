/*
Copyright (c) 2005 bayside

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation files 
(the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define DISKICON      0
#define TERMINALICON  1
#define TRASHBOXICON  2

/**
 ショートカットアイコン
*/
class ShortCut : public Window {
private:
	/** タイトル */
	String title;
	/** アイコンタイプ */
	int type;

protected:
	/** タイトルバーがクリックされたかどうか */
	virtual bool getTitlebarClicked(int px, int py)
	{
		return false;
	}
	
public:
	/** コンストラクタ */
	ShortCut();
	
	/** デストラクタ */
	virtual ~ShortCut();
	
	/** タイトルを設定する */
	void setTitle(const String& title);
	
	/**
	 アイコンタイプを設定する
	 @param type アイコンタイプ
	*/
	void setType(int type);
	
	/** ウィンドウ生成時に呼ばれる */
	virtual void addNotify();

	/** 再描画 */
	virtual void paint(Graphics* g);

	/** イベントハンドラ */
	virtual void processEvent(Event* event);
};
