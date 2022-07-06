#pragma once

#include "canvas.hpp"
#include "channel.hpp"
#include "apparatus.hpp"

namespace lvk
{
	struct app
	{
		static constexpr int WIDTH = 1280, HEIGHT = 720;
		canvas window{ WIDTH, HEIGHT, "first app" };
		apparatus device{ window };
		channel pipeline{ device, channel::default_pipeline_config_info(WIDTH, HEIGHT),
			"shaders/spv/test.vert.spv", "shaders/spv/test.frag.spv" };

		void run();
	};
}
