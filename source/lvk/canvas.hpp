#pragma once

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include <string>

namespace lvk
{
	class canvas
	{
		int width, height;
		std::string window_name;
		GLFWwindow* window;

		void init_window();

	public:
		canvas(int _width, int _height, const std::string& _window_name);
		~canvas();

		canvas(const canvas&) = delete;
		canvas& operator=(const canvas&) = delete;

		bool should_close();
	};
}
