#include "dewcin.h"


namespace dewcin
{
	HINSTANCE Window::hInstance;
	
	Window::Window(const char* s_title, int s_width, int s_height)
	{
		title = s_title;
		width = s_width;
		height = s_height;
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

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window_handle, &paint);

			// update the window's dimensions
			WindowDimensions window_dimensions = Renderer::getWindowDimensions(window_handle);
			Renderer::win32_copyBufferToWindow(&window->backbuffer, device_context, window_dimensions.width, window_dimensions.height);

			EndPaint(window_handle, &paint);
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

		Renderer::win32_resizeFrameBuffer(&backbuffer, width, height);

		window_class.style = CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = WindowCallback;
		window_class.hInstance = Window::hInstance;
		//window_class.hIcon;

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
			width,
			height,
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

				Renderer::render_background(&backbuffer, { 1.f, 0.f, 0.f });

				// Render the graphics_buffer
				HDC device_context = GetDC(window_handle);
				WindowDimensions dimensions = Renderer::getWindowDimensions(window_handle);
				Renderer::win32_copyBufferToWindow(
					&backbuffer,
					device_context,
					dimensions.width,
					dimensions.height
				);
				ReleaseDC(window_handle, device_context);
			}
		}
		else
		{
			OutputDebugStringA("Window handle is null\n");
		}
	}
}