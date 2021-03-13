#pragma once

#include <windows.h>

#include <thread>
#include <string>
#include <functional>


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

	union Rect
	{
		struct { int x, y, z, w; };
		struct { int x, y, width, height; };
		struct { int left, top, right, bottom; };
		struct { int min_x, min_y, max_x, max_y; };
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
		// Render a rectangle
		// window - the window on which to render
		// rect - x, y, width, height, where x,y - a point for the left-top corner and width,height - the size of the rectangle
		// color - color in float RGB
		static void FillRectangle(Window* window, const Rect& rect, RGBColor color);

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

		Renderer::BitmapBuffer graphics_buffer;

	public:
		// The background color of the window in float RGB
		// Gray color (0.5f, 0.5f, 0.5f) by default if unspecified
		RGBColor background_color;
		
		// if a function is provided, it will be called each time the window updates, before clearing the screen
		// (intended to be used as an update function before the rendering)
		std::function<void(Window*, float delta)> before_clear;
		// if a function is provided, it will be called each time after the window is cleared with the background color
		// (intended to be used as a main render function)
		std::function<void(Window*, float delta)> after_clear;

	private:
		const char* title;
		Dimensions size;
	
	public:
		// TODO : window background color, positioning, resize, fullscreen
		Window(const char* title, const Dimensions& size);
		~Window();

		inline const char* getTitle() { return title; }
		inline Dimensions getSize() { return size; }

	private:
		void start_window();
	};
}