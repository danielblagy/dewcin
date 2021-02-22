#pragma once

#include <windows.h>

#include <thread>


namespace dewcin
{
	class Window
	{
	private:
		static HINSTANCE hInstance;
		
		//WNDCLASSA window_class;
		std::thread window_thread;
		bool running;
	
	public:
		Window();
		~Window();

	private:
		void start_window();
	};
}