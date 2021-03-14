#include "dewcin.h"

DEWCIN_APP_ENTRY_POINT
{
	dewcin::Window test_window("A Test Dewcin Window", { 500, 720 });

	dewcin::Rect player = { 150, 150, 60, 120 };
	float accumulated_delta = 0.0f;
	float period_ms = 500.f;

	test_window.before_clear = [&](dewcin::Window* window, float delta)
	{
		//char char_buffer[256];
		//sprintf_s(char_buffer, "delta: %.2f\n", delta);
		//OutputDebugStringA(char_buffer);
		
		if (dewcin::Input::isKeyPressed(DC_A))
			OutputDebugStringA("A has been pressed!\n");
		
		accumulated_delta += delta;

		if (accumulated_delta >= period_ms)
		{
			player.x++;
			player.y++;

			accumulated_delta -= period_ms;
		}
	};
	
	// set window callbacks
	test_window.after_clear = [&](dewcin::Window* window, float delta)
	{
		dewcin::Renderer::FillRectangle(window, player, { 0.f, 1.f, 0.f });
	};
	
	return 0;
}