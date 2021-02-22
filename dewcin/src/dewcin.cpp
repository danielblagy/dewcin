#include "dewcin.h"


namespace dewcin
{

}

LRESULT CALLBACK WindowCallback(
	HWND window_handle,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	LRESULT result = 0;

	result = DefWindowProc(window_handle, message, wParam, lParam);

	return result;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) // NOTE: MSDN
{
	WNDCLASSA window_class = {};

	OutputDebugStringA("Hey!\n");

	//win32_resizeFrameBuffer(&backbuffer, screen_width, screen_height);

	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = WindowCallback;
	window_class.hInstance = hInstance;
	//window_class.hIcon;
	window_class.lpszClassName = "dewcin_WindowClass";

	if (!RegisterClassA(&window_class))
	{
		OutputDebugStringA("Failed to create window (class did not register)\n");
		return -1;
	}

	HWND window_handle = CreateWindowExA(
		0,
		window_class.lpszClassName,
		"dewcin",
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
		while (true)//running)
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

	return 0;
}