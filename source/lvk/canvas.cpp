#include <stdexcept>
#include "canvas.hpp"

namespace lvk
{
	void canvas::init_window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	}

	canvas::canvas(int _width, int _height, const std::string& _window_name)
		: width{ _width }, height{ _height }, window_name{ _window_name }
	{
		init_window();
	}

	canvas::~canvas()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool canvas::should_close()
	{
		return glfwWindowShouldClose(window);
	}
	void canvas::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}

	}
}

