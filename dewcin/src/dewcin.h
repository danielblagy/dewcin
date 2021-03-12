#pragma once

#include <windows.h>

#include <thread>

#include <string>


#define DEWCIN_APP_ENTRY_POINT INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)


namespace dewcin
{
	static int (*entry_point)(void) = 0;
	
	class Window
	{
	private:
		// TODO : move to Gui
		static HINSTANCE hInstance;
		
		std::thread window_thread;
		bool running;

		const char* title;
	
	public:
		Window(const char* title);
		~Window();

		inline const char* get_title() { return title; }

	private:
		void start_window();

		friend LRESULT CALLBACK WindowCallback(
			HWND window_handle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);
	};
}