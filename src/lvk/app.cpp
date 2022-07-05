#include "app.hpp"

namespace lvk
{
	void app::run()
	{
		while (!_canvas.should_close())
		{
			glfwPollEvents();
		}
	}
}