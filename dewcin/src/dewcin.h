#pragma once

#include <windows.h>

#include <thread>

#include <string>


#define DEWCIN_APP_ENTRY_POINT INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)


namespace dewcin
{
	struct Win32_BitmapBuffer
	{
		int width, height;
		BITMAPINFO info;
		void* memory;
		int pitch;	// in bytes
	};

	struct WindowDimensions
	{
		int width, height;
	};

	struct RGBColor
	{
		float red;
		float green;
		float blue;
	};

	#define BYTES_PER_PIXEL 4

	class Renderer
	{
	public:
		static WindowDimensions getWindowDimensions(HWND window_handle)
		{
			RECT client_rect;
			GetClientRect(window_handle, &client_rect);

			return { client_rect.right - client_rect.left, client_rect.bottom - client_rect.top };
		}


		// resizes the Device Independent Buffer (DIB) section
		static void win32_resizeFrameBuffer(Win32_BitmapBuffer* buffer, int width, int height)
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

			int bitmap_memory_size = buffer->width * buffer->height * BYTES_PER_PIXEL;
			buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			buffer->pitch = buffer->width * BYTES_PER_PIXEL;
		}

		static void win32_copyBufferToWindow(Win32_BitmapBuffer* buffer, HDC device_context, int window_width, int window_height)
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

		static void render_background(Win32_BitmapBuffer* buffer, RGBColor color)
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

	private:
		static uint32_t round_float_to_uint32(float value)
		{
			return (uint32_t)(value + 0.5f);
		}

		static int32_t round_float_to_int32(float value)
		{
			return (int32_t)(value + 0.5f);
		}
	};
	
	class Window
	{
	private:
		// TODO : move to Gui
		static HINSTANCE hInstance;
		
		std::thread window_thread;
		bool running;

		const char* title;
		int width, height;

		Win32_BitmapBuffer backbuffer;
	
	public:
		Window(const char* title, int width, int height);
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