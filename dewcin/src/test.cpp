#include "dewcin.h"

DEWCIN_APP_ENTRY_POINT
{
	dewcin::Window test_window("A Test Dewcin Window", { 500, 720 });

	dewcin::Rect player = { 150, 150, 60, 120 };

	test_window.before_clear = [&](dewcin::Window* window)
	{
		player.x++;
		player.y++;
	};
	
	// set window callbacks
	test_window.after_clear = [&](dewcin::Window* window)
	{
		dewcin::Renderer::FillRectangle(window, player, { 0.f, 1.f, 0.f });
	};
	
	return 0;
}