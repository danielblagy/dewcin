#pragma once

#include <windows.h>
#include <windowsx.h>

#include <thread>
#include <string>
#include <functional>


#define DEWCIN_APP_ENTRY_POINT INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)


#define DC_MAX_KEYS 52

#define DC_A			0
#define DC_B			1
#define DC_C			2
#define DC_D			3
#define DC_E			4
#define DC_F			5
#define DC_G			6
#define DC_H			7
#define DC_I			8
#define DC_J			9
#define DC_K			10
#define DC_L			11
#define DC_M			12
#define DC_N			13
#define DC_O			14
#define DC_P			15
#define DC_Q			16
#define DC_R			17
#define DC_S			18
#define DC_T			19
#define DC_U			20
#define DC_V			21
#define DC_W			22
#define DC_X			23
#define DC_Y			24
#define DC_Z			25

#define DC_UP			26
#define DC_DOWN			27
#define DC_LEFT			28
#define DC_RIGHT		29

#define DC_0			30
#define DC_1			31
#define DC_2			32
#define DC_3			33
#define DC_4			34
#define DC_5			35
#define DC_6			36
#define DC_7			37
#define DC_8			38
#define DC_9			39
#define DC_MINUS		40
#define DC_PLUS			41

#define DC_SHIFT		42
#define DC_CONTROL		43
#define DC_ALT			44
#define DC_SPACE		45
#define DC_ESCAPE		46
#define DC_CAPSLOCK		47
#define DC_TAB			48
#define DC_ENTER		49
#define DC_BACKSPACE	50
#define DC_TILDE		51

#define DC_MAX_MOUSE_BUTTONS 5

#define DC_MOUSE_LEFT	0
#define DC_MOUSE_RIGHT	1
#define DC_MOUSE_MIDDLE	2
#define DC_MOUSE_X1		3
#define DC_MOUSE_X2		4


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

		friend class Component;
		friend class Button;

	private:
		// TODO : move to Gui
		static HINSTANCE hInstance;

		HWND window_handle = 0;
		
		std::thread window_thread;
		bool running;

		Renderer::BitmapBuffer graphics_buffer;

		bool init_occurred = false;

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
		// TODO : just use plain old int, and add x & y
		Dimensions size;
	
	public:
		// TODO : window background color, positioning, resize, fullscreen
		Window(const char* title, const Dimensions& size);
		~Window();

		void add(Component* component);

		inline const char* getTitle() { return title; }
		inline Dimensions getSize() { return size; }

	private:
		void start_window();
	};

	// TODO : extensive testing of the keyboard input polling
	// TODO : extensive testing of the mouse input polling
	class Input
	{
		friend LRESULT CALLBACK WindowCallback(
			HWND window_handle,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);

	public:
		struct KeyState
		{
			uint8_t is_down : 4;
			uint8_t was_down : 4;
		};

		struct KeyboardInputMap
		{
			KeyState keys[DC_MAX_KEYS];
		};

		struct ButtonState
		{
			uint8_t is_down : 4;
			uint8_t was_down : 4;
		};
		
		struct Position
		{
			int x, y;
		};
		
		struct MouseInputMap
		{
			ButtonState buttons[DC_MAX_MOUSE_BUTTONS];
			Position position;
		};
	
	private:
		static KeyboardInputMap keyboard;
		static MouseInputMap mouse;
	
	public:
		// Returns an Input::KeyState of the key
		static KeyState getKeyState(unsigned int keycode);
		
		// Returns true if a key is being pressed down, false otherwise
		static bool isKeyPressed(unsigned int keycode);
		
		// Returns true if a key has stopped being pressed down, false otherwise
		static bool isKeyReleased(unsigned int keycode);

		// Returns true if a key has just been pressed down, false otherwise
		static bool wasKeyHit(unsigned int keycode);

		// Returns the x- and y-coordinates of the mouse pointer
		static Position getMousePosition();

		// Returns true if a mouse button is being pressed down, false otherwise
		static bool isMouseButtonPressed(unsigned int button_code);

		// Returns true if a mouse button has stopped being pressed down, false otherwise
		static bool isMouseButtonReleased(unsigned int button_code);

		// Returns true if a mouse button has just been pressed down, false otherwise
		static bool wasMouseButtonHit(unsigned int button_code);

	private:
		static void process_keyboard_input(uint32_t VKCode, bool was_down, bool is_down);

		static void process_mouse_input(WPARAM wParam, LPARAM lParam);
		static void update_mouse_position(LPARAM lParam);
	};

	class Component
	{
		friend Window;
	
	protected:
		const char* text;
		int x, y, width, height;

	protected:
		Component(const char* text, int x, int y, int width, int height);

	protected:
		virtual void create(Window* window) = 0;
	};
	
	class Button : public Component
	{
		friend Window;

	private:
		HWND button_handle = 0;
	
	public:
		Button(const char* text, int x, int y, int width, int height);
	
	private:
		virtual void create(Window* window) override;
	};
}