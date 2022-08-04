#pragma once

#include "window_wrp.hpp"
#include "pipeline_wrp.hpp"
#include "device_wrp.hpp"
#include "swap_chain_wrp.hpp"

#include "memory"
#include "vector"

namespace lvk
{
	class app
	{
		public:
		static constexpr int WIDTH = 1280, HEIGHT = 720;

		app();
		~app();
		app(const app&) = delete;
		app &operator=(const app&) = delete;

		void run();

		private:
		window_wrp window{ WIDTH, HEIGHT, "first app" };
		device_wrp device{ window };
		swap_chain_wrp swap_chain{device, window.get_extent()};
//		pipeline_wrp pipeline{
//			device, pipeline_wrp::default_pipeline_config_info(WIDTH, HEIGHT),
//			"shaders/spv/test.vert.spv", "shaders/spv/test.frag.spv" };
		std::unique_ptr<pipeline_wrp> pipeline;
		VkPipelineLayout pipeline_layout;
		std::vector<VkCommandBuffer> command_buffers;

		void create_pipeline_layout();
		void create_pipeline();
		void create_command_buffers();
		void draw_frame();
	};
}
