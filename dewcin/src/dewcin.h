#pragma once

#include <windows.h>

#include <thread>

#include <string>


#define DEWCIN_APP_ENTRY_POINT INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)


namespace dewcin
{
	union Dimensions
	{
		struct { int width, height; };
		struct { int x, y; };
	};
	
	union RGBColor
	{
		struct { float red, green, blue; };
		struct { float r, g, b; };
	};

	class Renderer
	{
		friend LRESULT CALLBACK WindowCallback(
			HWND window_handle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

		friend class Window;
	
	private:
		struct BitmapBuffer
		{
			int width, height;
			BITMAPINFO info;
			void* memory;
			int pitch;	// in bytes
		};

	private:
		static const int bytes_per_pixel = 4;

	// TODO : add public functions for drawing other shapes
	public:
		// TODO : make BitmapBuffer private and require dewcin::Window* here instead
		static void FillRectangle(BitmapBuffer* buffer, int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y, RGBColor color);

	private:
		static Dimensions getWindowDimensions(HWND window_handle);

		// resizes the Device Independent Buffer (DIB) section
		static void resize_frame_buffer(BitmapBuffer* buffer, int width, int height);

		static void copy_buffer_to_window(BitmapBuffer* buffer, HDC device_context, int window_width, int window_height);

		// TODO : should this be a public api function ??
		static void render_background(BitmapBuffer* buffer, RGBColor color);

	private:
		static inline uint32_t round_float_to_uint32(float value)
		{
			return (uint32_t)(value + 0.5f);
		}

		static inline int32_t round_float_to_int32(float value)
		{
			return (int32_t)(value + 0.5f);
		}
	};
	
	class Window
	{
		friend LRESULT CALLBACK WindowCallback(
			HWND window_handle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);
		
		friend class Renderer;

	private:
		// TODO : move to Gui
		static HINSTANCE hInstance;
		
		std::thread window_thread;
		bool running;

		const char* title;
		int width, height;

		Renderer::BitmapBuffer graphics_buffer;
	
	public:
		Window(const char* title, int width, int height);
		~Window();

		inline const char* get_title() { return title; }

	private:
		void start_window();
	};
}