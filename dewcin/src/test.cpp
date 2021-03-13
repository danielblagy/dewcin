#include "dewcin.h"

DEWCIN_APP_ENTRY_POINT
{
	dewcin::Window test_window("A Test Dewcin Window", { 500, 720 });

	// set window callbacks
	test_window.after_clear = [](dewcin::Window* window)
	{
		dewcin::Renderer::FillRectangle(window, { 200, 200, 150, 80 }, { 0.f, 1.f, 0.f });
	};
	
	return 0;
}