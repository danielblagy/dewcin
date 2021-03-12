#include "dewcin.h"


/*INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	// TODO : call the user-defined entry point here

	if (dewcin::entry_point)
	{
		return dewcin::entry_point();
	}
	else
	{
		OutputDebugStringA("Set an entry point function by calling dewcin::Gui::SetEntryPoint()\n");
	}

	return 0;
}*/

namespace dewcin
{
	//int (*entry_point)(void) = 0;
	
	HINSTANCE Window::hInstance;
	
	Window::Window(const char* s_title)
	{
		title = s_title;
		running = true;
		window_thread = std::thread(&Window::start_window, this);
	}

	Window::~Window()
	{
		window_thread.join();
	}
	
	// TODO : resolve the separate thread issue
	LRESULT CALLBACK WindowCallback(
		HWND window_handle,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	)
	{
		LRESULT result = 0;

		Window* window = (Window*)GetWindowLongPtr(window_handle, GWLP_USERDATA);
		if (window)
			OutputDebugStringA(window->get_title());

		switch (message)
		{
		case WM_CLOSE:
		{
			window->running = false;
			OutputDebugStringA("Window close\n");
		} break;

		case WM_DESTROY:
		{
			window->running = false;
			OutputDebugStringA("Window destroy\n");
		} break;
		default:
		{
			result = DefWindowProc(window_handle, message, wParam, lParam);
		} break;
		}

		return result;
	}
	
	void Window::start_window()
	{
		WNDCLASSA window_class = {};

		OutputDebugStringA("Hey!\n");

		//win32_resizeFrameBuffer(&backbuffer, screen_width, screen_height);

		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = WindowCallback;
		window_class.hInstance = Window::hInstance;
		//window_class.hIcon;
		//window_class.lpszClassName = "dewcin_WindowClass";

		std::string window_class_name = "dewcin_" + std::string(title);

		window_class.lpszClassName = window_class_name.c_str();

		if (!RegisterClassA(&window_class))
		{
			OutputDebugStringA("Failed to create window (class did not register)\n");
			return;
		}

		HWND window_handle = CreateWindowExA(
			0,
			window_class.lpszClassName,
			title,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			hInstance,
			0
		);

		if (window_handle)
		{
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR) this);
			OutputDebugStringA("INIT\n");
			while (running)
			{
				// process windows messages
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
						;//running = false;

					TranslateMessage(&message);
					DispatchMessage(&message);		// Send message to the WindowProc
				}

				/*BitmapBuffer graphics_buffer = {};
				graphics_buffer.memory = backbuffer.memory;
				graphics_buffer.width = backbuffer.width;
				graphics_buffer.height = backbuffer.height;
				graphics_buffer.pitch = backbuffer.pitch;


				HDC device_context = GetDC(window_handle);
				WindowDimensions dimensions = getWindowDimensions(window_handle);
				win32_copyBufferToWindow(
					&backbuffer,
					device_context,
					dimensions.width,
					dimensions.height
				);
				ReleaseDC(window_handle, device_context);*/
			}
		}
		else
		{
			OutputDebugStringA("Window handle is null\n");
		}
	}
}