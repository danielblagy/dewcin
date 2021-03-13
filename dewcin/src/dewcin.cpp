#include "dewcin.h"


namespace dewcin
{
	// TODO : make BitmapBuffer private and require dewcin::Window* here instead
	void Renderer::FillRectangle(BitmapBuffer* buffer, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, RGBColor color)
	{
		// clipping
		if (min_x < 0)				min_x = 0;
		if (min_y < 0)				min_y = 0;
		if (max_x > buffer->width)	max_x = buffer->width;
		if (max_y > buffer->height)	max_y = buffer->height;

		// convert float rgb color to uint representation
		uint32_t raw_color = (round_float_to_uint32(color.red * 255.0f) << 16) |
			(round_float_to_uint32(color.green * 255.0f) << 8) |
			(round_float_to_uint32(color.blue * 255.0f) << 0);

		uint8_t* row = (uint8_t*)buffer->memory + min_x * bytes_per_pixel + min_y * buffer->pitch;
		for (int y = min_y; y < max_y; y++)
		{
			uint32_t* pixel = (uint32_t*)row;
			for (int x = min_x; x < max_x; x++)
			{
				*pixel++ = raw_color;
			}
			row += buffer->pitch;
		}
	}

	Dimensions Renderer::getWindowDimensions(HWND window_handle)
	{
		RECT client_rect;
		GetClientRect(window_handle, &client_rect);

		return { client_rect.right - client_rect.left, client_rect.bottom - client_rect.top };
	}


	// resizes the Device Independent Buffer (DIB) section
	void Renderer::resize_frame_buffer(BitmapBuffer* buffer, int width, int height)
	{
		if (buffer->memory)
		{
			VirtualFree(buffer->memory, 0, MEM_RELEASE);
		}

		buffer->width = width;
		buffer->height = height;

		buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
		buffer->info.bmiHeader.biWidth = buffer->width;
		buffer->info.bmiHeader.biHeight = -(buffer->height);
		buffer->info.bmiHeader.biPlanes = 1;
		buffer->info.bmiHeader.biBitCount = 32;
		buffer->info.bmiHeader.biCompression = BI_RGB;

		int bitmap_memory_size = buffer->width * buffer->height * bytes_per_pixel;
		buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		buffer->pitch = buffer->width * bytes_per_pixel;
	}

	void Renderer::copy_buffer_to_window(BitmapBuffer* buffer, HDC device_context, int window_width, int window_height)
	{
		StretchDIBits(
			device_context,
			0, 0, window_width, window_height,
			0, 0, buffer->width, buffer->height,
			buffer->memory,
			&(buffer->info),
			DIB_RGB_COLORS,
			SRCCOPY
		);
	}

	// TODO : should this be a public api function ??
	void Renderer::render_background(BitmapBuffer* buffer, RGBColor color)
	{
		uint32_t raw_color = (round_float_to_uint32(color.red * 255.0f) << 16) |
			(round_float_to_uint32(color.green * 255.0f) << 8) |
			(round_float_to_uint32(color.blue * 255.0f) << 0);

		uint8_t* row = (uint8_t*)buffer->memory;
		for (int y = 0; y < buffer->height; y++)
		{
			uint32_t* pixel = (uint32_t*)row;
			for (int x = 0; x < buffer->width; x++)
			{
				*pixel++ = raw_color;
			}
			row += buffer->pitch;
		}
	}
	
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

			// update when resized (stretches the image)
			// TODO : maybe do a resize_frame_buffer ??
			Dimensions window_dimensions = Renderer::getWindowDimensions(window_handle);
			Renderer::copy_buffer_to_window(&window->graphics_buffer, device_context, window_dimensions.width, window_dimensions.height);

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

		Renderer::resize_frame_buffer(&graphics_buffer, width, height);

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

				Renderer::render_background(&graphics_buffer, { 1.f, 0.f, 0.f });
				//Renderer::render_background(&graphics_buffer, { 0.f, 0.f, 1.f });

				Renderer::FillRectangle(&graphics_buffer, 200, 200, 350, 280, { 0.f, 1.f, 0.f });

				// Render the graphics_buffer
				HDC device_context = GetDC(window_handle);
				Dimensions dimensions = Renderer::getWindowDimensions(window_handle);
				Renderer::copy_buffer_to_window(
					&graphics_buffer,
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