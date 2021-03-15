#include "dewcin.h"


namespace dewcin
{
	// Renderer
	
	void Renderer::FillRectangle(Window* window, const Rect& rect, RGBColor color)
	{
		BitmapBuffer* buffer = &window->graphics_buffer;
		Rect draw_rect = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
		
		// clipping
		if (draw_rect.min_x < 0)				draw_rect.min_x = 0;
		if (draw_rect.min_y < 0)				draw_rect.min_y = 0;
		if (draw_rect.max_x > buffer->width)	draw_rect.max_x = buffer->width;
		if (draw_rect.max_y > buffer->height)	draw_rect.max_y = buffer->height;

		// convert float rgb color to uint representation
		uint32_t raw_color = (round_float_to_uint32(color.red * 255.0f) << 16) |
			(round_float_to_uint32(color.green * 255.0f) << 8) |
			(round_float_to_uint32(color.blue * 255.0f) << 0);

		uint8_t* row = (uint8_t*)buffer->memory + draw_rect.min_x * bytes_per_pixel + draw_rect.min_y * buffer->pitch;
		for (int y = draw_rect.min_y; y < draw_rect.max_y; y++)
		{
			uint32_t* pixel = (uint32_t*)row;
			for (int x = draw_rect.min_x; x < draw_rect.max_x; x++)
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
	
	// Window
	
	HINSTANCE Window::hInstance;
	
	Window::Window(const char* s_title, const Dimensions& s_size)
	{
		title = s_title;
		size = s_size;
		running = true;
		window_thread = std::thread(&Window::start_window, this);

		// gray color by default if unspecified
		background_color = { 0.5f, 0.5f, 0.5f };
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
		//if (window)
		//	OutputDebugStringA(window->title);

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

		// keyboard input handling
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32_t VKCode = wParam;

			bool was_down = (lParam & (1 << 30)) != 0;
			bool is_down = (lParam & (1 << 31)) == 0;
			if (was_down != is_down)
			{
				// handle the close operation with alt+F4
				bool alt_key_was_down = (lParam & (1 << 29)) != 0;
				if ((VKCode == VK_F4) && alt_key_was_down)
				{
					window->running = false;
				}
			}

			Input::process_keyboard_input(VKCode, was_down, is_down);
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		{
			Input::process_mouse_input(wParam, lParam);
		} break;

		case WM_MOUSEMOVE:
		{
			Input::update_mouse_position(lParam);
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

		Renderer::resize_frame_buffer(&graphics_buffer, size.width, size.height);

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
			size.width,
			size.height,
			0,
			0,
			hInstance,
			0
		);

		if (window_handle)
		{
			OutputDebugStringA("INIT\n");
			
			// Set user data for the win32 window callback function
			SetWindowLongPtr(window_handle, GWLP_USERDATA, (LONG_PTR) this);

			// init the clock
			LARGE_INTEGER cpu_frequency;
			QueryPerformanceFrequency(&cpu_frequency);

			LARGE_INTEGER last_counter;
			QueryPerformanceCounter(&last_counter);

			uint64_t last_cycle_count = __rdtsc();

			while (running)
			{
				// handle time delta
				uint64_t current_cycle_count = __rdtsc();

				LARGE_INTEGER current_counter;
				QueryPerformanceCounter(&current_counter);

				uint64_t cycles_elapsed = current_cycle_count - last_cycle_count;
				int64_t counter_elapsed = current_counter.QuadPart - last_counter.QuadPart;

				float delta = (float)counter_elapsed / (float)cpu_frequency.QuadPart;	// in seconds

				last_cycle_count = current_cycle_count;
				last_counter = current_counter;
				
				// process windows messages
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
						;//running = false;

					TranslateMessage(&message);
					DispatchMessage(&message);		// Send message to the WindowProc
				}

				// update & render
				if (before_clear)
					before_clear(this, delta);
				
				Renderer::render_background(&graphics_buffer, background_color);

				if (after_clear)
					after_clear(this, delta);

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

	// Input
	
	Input::KeyboardInputMap Input::keyboard;
	Input::MouseInputMap Input::mouse;
	
	// TODO : add test for validity of the provided keycode ??

	Input::KeyState Input::getKeyState(unsigned int keycode)
	{
		return keyboard.keys[keycode];
	}

	bool Input::isKeyPressed(unsigned int keycode)
	{
		return keyboard.keys[keycode].is_down;
	}

	bool Input::isKeyReleased(unsigned int keycode)
	{
		return !keyboard.keys[keycode].is_down && keyboard.keys[keycode].was_down;
	}

	bool Input::wasKeyHit(unsigned int keycode)
	{
		return keyboard.keys[keycode].is_down && !keyboard.keys[keycode].was_down;
	}

	Input::Position Input::getMousePosition()
	{
		return mouse.position;
	}
	
	bool Input::isMouseButtonPressed(unsigned int button_code)
	{
		return mouse.buttons[button_code].is_down;
	}

	bool Input::isMouseButtonReleased(unsigned int button_code)
	{
		return !mouse.buttons[button_code].is_down && mouse.buttons[button_code].was_down;
	}

	bool Input::wasMouseButtonHit(unsigned int button_code)
	{
		return mouse.buttons[button_code].is_down && !mouse.buttons[button_code].was_down;
	}
	
	void Input::process_keyboard_input(uint32_t VKCode, bool was_down, bool is_down)
	{
		if (was_down != is_down)
		{
			if (VKCode >= 'A' && VKCode <= 'Z')
			{
				uint8_t dc_keycode = VKCode - 'A';
				keyboard.keys[dc_keycode].is_down = is_down;
				keyboard.keys[dc_keycode].was_down = was_down;
			}
			else if (VKCode == VK_UP)
			{
				keyboard.keys[DC_UP].is_down = is_down;
				keyboard.keys[DC_UP].was_down = was_down;
			}
			else if (VKCode == VK_DOWN)
			{
				keyboard.keys[DC_DOWN].is_down = is_down;
				keyboard.keys[DC_DOWN].was_down = was_down;
			}
			else if (VKCode == VK_LEFT)
			{
				keyboard.keys[DC_LEFT].is_down = is_down;
				keyboard.keys[DC_LEFT].was_down = was_down;
			}
			else if (VKCode == VK_RIGHT)
			{
				keyboard.keys[DC_RIGHT].is_down = is_down;
				keyboard.keys[DC_RIGHT].was_down = was_down;
			}
			else if (VKCode >= '0' && VKCode <= '9')
			{
				uint8_t dc_keycode = VKCode - '0' + DC_0;
				keyboard.keys[dc_keycode].is_down = is_down;
				keyboard.keys[dc_keycode].was_down = was_down;
			}
			else if (VKCode == VK_OEM_MINUS)
			{
				keyboard.keys[DC_MINUS].is_down = is_down;
				keyboard.keys[DC_MINUS].was_down = was_down;
			}
			else if (VKCode == VK_OEM_PLUS)
			{
				keyboard.keys[DC_PLUS].is_down = is_down;
				keyboard.keys[DC_PLUS].was_down = was_down;
			}
			else if (VKCode == VK_SHIFT)
			{
				keyboard.keys[DC_SHIFT].is_down = is_down;
				keyboard.keys[DC_SHIFT].was_down = was_down;
			}
			else if (VKCode == VK_CONTROL)
			{
				keyboard.keys[DC_CONTROL].is_down = is_down;
				keyboard.keys[DC_CONTROL].was_down = was_down;
			}
			else if (VKCode == VK_MENU)
			{
				keyboard.keys[DC_ALT].is_down = is_down;
				keyboard.keys[DC_ALT].was_down = was_down;
			}
			else if (VKCode == VK_SPACE)
			{
				keyboard.keys[DC_SPACE].is_down = is_down;
				keyboard.keys[DC_SPACE].was_down = was_down;
			}
			else if (VKCode == VK_ESCAPE)
			{
				keyboard.keys[DC_ESCAPE].is_down = is_down;
				keyboard.keys[DC_ESCAPE].was_down = was_down;
			}
			else if (VKCode == VK_CAPITAL)
			{
				keyboard.keys[DC_CAPSLOCK].is_down = is_down;
				keyboard.keys[DC_CAPSLOCK].was_down = was_down;
			}
			else if (VKCode == VK_TAB)
			{
				keyboard.keys[DC_TAB].is_down = is_down;
				keyboard.keys[DC_TAB].was_down = was_down;
			}
			else if (VKCode == VK_RETURN)
			{
				keyboard.keys[DC_ENTER].is_down = is_down;
				keyboard.keys[DC_ENTER].was_down = was_down;
			}
			else if (VKCode == VK_BACK)
			{
				keyboard.keys[DC_BACKSPACE].is_down = is_down;
				keyboard.keys[DC_BACKSPACE].was_down = was_down;
			}
			else if (VKCode == VK_OEM_3)
			{
				keyboard.keys[DC_TILDE].is_down = is_down;
				keyboard.keys[DC_TILDE].was_down = was_down;
			}
		}
	}

	void Input::process_mouse_input(WPARAM wParam, LPARAM lParam)
	{
		// update every button state
		mouse.buttons[DC_MOUSE_LEFT].was_down = mouse.buttons[DC_MOUSE_LEFT].is_down;
		mouse.buttons[DC_MOUSE_RIGHT].was_down = mouse.buttons[DC_MOUSE_RIGHT].is_down;
		mouse.buttons[DC_MOUSE_MIDDLE].was_down = mouse.buttons[DC_MOUSE_MIDDLE].is_down;
		mouse.buttons[DC_MOUSE_X1].was_down = mouse.buttons[DC_MOUSE_X1].is_down;
		mouse.buttons[DC_MOUSE_X2].was_down = mouse.buttons[DC_MOUSE_X2].is_down;
		
		mouse.buttons[DC_MOUSE_LEFT].is_down = wParam & MK_LBUTTON;
		mouse.buttons[DC_MOUSE_RIGHT].is_down = wParam & MK_RBUTTON;
		mouse.buttons[DC_MOUSE_MIDDLE].is_down = wParam & MK_MBUTTON;
		mouse.buttons[DC_MOUSE_X1].is_down = wParam & MK_XBUTTON1;
		mouse.buttons[DC_MOUSE_X2].is_down = wParam & MK_XBUTTON2;

		update_mouse_position(lParam);
	}

	void Input::update_mouse_position(LPARAM lParam)
	{
		mouse.position.x = GET_X_LPARAM(lParam);
		mouse.position.y = GET_Y_LPARAM(lParam);
	}
}