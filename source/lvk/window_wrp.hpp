#pragma once

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include <string>

namespace lvk
{
	class window_wrp
	{
		unsigned int width, height;
		std::string window_name;
		GLFWwindow* window;

		void init_window();

	public:
		window_wrp(unsigned int _width, unsigned int _height, const std::string& _window_name);
		~window_wrp();

		window_wrp(const window_wrp&) = delete;
		window_wrp& operator=(const window_wrp&) = delete;

		bool should_close();
		VkExtent2D get_extent();
		void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);
	};
}
