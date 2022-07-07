#include <stdexcept>
#include "window_wrp.hpp"

namespace lvk
{
	void window_wrp::init_window()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	}

	window_wrp::window_wrp(int _width, int _height, const std::string& _window_name)
		: width{ _width }, height{ _height }, window_name{ _window_name }
	{
		init_window();
	}

	window_wrp::~window_wrp()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool window_wrp::should_close()
	{
		return glfwWindowShouldClose(window);
	}
	void window_wrp::create_window_surface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window get_surface");
		}

	}
}

