#pragma once

#include <windows.h>

#include <thread>


namespace dewcin
{
	class Window
	{
	private:
		static HINSTANCE hInstance;
		
		static std::thread window_thread;
		static bool running;

		const char* title;
	
	public:
		Window(const char* title);
		~Window();

	private:
		void start_window();

		static LRESULT CALLBACK WindowCallback(
			HWND window_handle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);
	};
}