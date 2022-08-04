#include "app.hpp"

#include <stdexcept>

namespace lvk
{
	app::app() {
		create_pipeline_layout();
		create_pipeline();
		create_command_buffers();
	}

	app::~app() {
		vkDestroyPipelineLayout(device.get_device(), pipeline_layout, nullptr);
	}

	void app::run()
	{
		while (!window.should_close())
		{
			glfwPollEvents();
		}
	}

	void app::create_pipeline_layout()
	{
		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		if(vkCreatePipelineLayout(device.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout)
		!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}
	void app::create_pipeline()
	{
		auto pipeline_config = pipeline->default_pipeline_config_info(swap_chain.width(), swap_chain.height());
		pipeline_config.render_pass = swap_chain.get_render_pass();
		pipeline_config.pipeline_layout = pipeline_layout;
		pipeline = std::make_unique<pipeline_wrp>(device, pipeline_config,
			"shaders/spv/test.vert.spv", "shaders/spv/test.frag.spv");
	}
	void app::create_command_buffers()
	{

	}
	void app::draw_frame()
	{

	}
}