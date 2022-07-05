#pragma once

#include "canvas.hpp"

namespace lvk
{
	struct app
	{
		static constexpr int width = 1280, height = 720;
		canvas _canvas{ width, height, "first app" };

		void run();
	};
}
