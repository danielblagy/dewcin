#include "dewcin.h"

DEWCIN_APP_ENTRY_POINT
{
	dewcin::Window test_window("A Test Dewcin Window", { 500, 720 });

	dewcin::Rect player = { 150, 150, 60, 120 };
	float player_x = 150.f, player_y = 150.f, player_width = 60.f, player_height = 120.f;

	test_window.before_clear = [&](dewcin::Window* window, float delta)
	{
		char char_buffer[256];
		//sprintf_s(char_buffer, "delta: %f\n", delta);
		//OutputDebugStringA(char_buffer);

		dewcin::Input::Position mouse_pos = dewcin::Input::getMousePosition();
		sprintf_s(char_buffer, "mouse pos: %d  %d\n", mouse_pos.x, mouse_pos.y);
		OutputDebugStringA(char_buffer);

		if (dewcin::Input::isMouseButtonPressed(DC_MOUSE_RIGHT))
		{
			OutputDebugStringA("right mouse button is pressed!\n");
		}
		
		if (dewcin::Input::isKeyPressed(DC_A))
			player_x -= 100.f * delta;
		else if (dewcin::Input::isKeyPressed(DC_D))
			player_x += 100.f * delta;

		if (dewcin::Input::isKeyPressed(DC_W))
			player_y -= 100.f * delta;
		else if (dewcin::Input::isKeyPressed(DC_S))
			player_y += 100.f * delta;
	};
	
	// set window callbacks
	test_window.after_clear = [&](dewcin::Window* window, float delta)
	{
		dewcin::Renderer::FillRectangle(
			window,
			{ static_cast<int>(player_x), static_cast<int>(player_y), static_cast<int>(player_width), static_cast<int>(player_height) },
			{ 0.f, 1.f, 0.f }
		);
	};
	
	return 0;
}