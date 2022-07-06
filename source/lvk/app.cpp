#include "app.hpp"

namespace lvk
{
	void app::run()
	{
		while (!window.should_close())
		{
			glfwPollEvents();
		}
	}
}