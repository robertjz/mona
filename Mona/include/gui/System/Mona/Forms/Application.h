// This software is in the public domain.
// There are no restrictions on any sort of usage of this software.

#ifndef __SYSTEM_MONA_FORMS_APPLICATION_H__
#define __SYSTEM_MONA_FORMS_APPLICATION_H__

#ifdef MONA
#include <monapi.h>
#define MAIN_ARGS List<char*>* pekoe
#define SET_MAIN_CLASS(T) int MonaMain(MAIN_ARGS) { \
	System::Mona::Forms::Application::__main<T>(pekoe); return 0; }
#else
#define MAIN_ARGS int argc, char* argv[]
#define SET_MAIN_CLASS(T) int main(MAIN_ARGS) { \
	System::Mona::Forms::Application::__main<T>(argc, argv); return 0; }
#endif

#include <gui/System/Pointer.h>
#include <gui/System/String.h>
#include <gui/System/Collections/ArrayList.h>

namespace System { namespace Mona { namespace Forms
{
	class Form;
	class Control;
	
	class Application
	{
	private:
		static _P<System::Collections::ArrayList<_P<Form> > > forms;
		static _P<Control> prevControl;
		static _P<Form> mainForm;
		static bool isExited;
		static int mouseButtons;
		
		static void Initialize();
		static void Dispose();
		
	public:
		static unsigned char* defaultFontData;
		
		static void Run();
		static void Run(_P<Form> form);
		static void ProcessEvent(unsigned int message, unsigned int arg1, unsigned int arg2, unsigned int arg3);
	    static void Exit();
		
		static void AddForm(_P<Form> f);
		static void RemoveForm(_P<Form> f);
		static _P<Form> FindForm(int x, int y);
		
		template <class T> static void __main(MAIN_ARGS)
		{
#ifdef MONA
			int len = pekoe->size();
			_A<String> args(len);
			for (int i = 0; i < len; i++) args[i] = pekoe->get(i);
#else
			_A<String> args(argc);
			for (int i = 0; i < argc; i++) args[i] = argv[i];
#endif
			
			Application::Initialize();
			T::Main(args);
			Application::Dispose();
		}
	};
}}}

#endif  // __SYSTEM_MONA_FORMS_APPLICATION_H__
