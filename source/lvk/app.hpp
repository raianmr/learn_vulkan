#pragma once

#include "window_wrp.hpp"
#include "pipeline_wrp.hpp"
#include "device_wrp.hpp"

namespace lvk
{
	struct app
	{
		static constexpr int WIDTH = 1280, HEIGHT = 720;
		window_wrp window{ WIDTH, HEIGHT, "first app" };
		device_wrp device{ window };
		pipeline_wrp pipeline{ device, pipeline_wrp::default_pipeline_config_info(WIDTH, HEIGHT),
			"shaders/spv/test.vert.spv", "shaders/spv/test.frag.spv" };

		void run();
	};
}
