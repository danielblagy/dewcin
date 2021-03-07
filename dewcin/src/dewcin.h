#pragma once

#include <windows.h>

#include <thread>

#include <string>


namespace dewcin
{
	class Window
	{
	private:
		static HINSTANCE hInstance;
		
		std::thread window_thread;
		bool running;

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