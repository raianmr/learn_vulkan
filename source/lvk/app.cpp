#include "app.hpp"

#include <stdexcept>
#include <array>

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
			draw_frame();
		}

		vkDeviceWaitIdle(device.get_device());
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
		command_buffers.resize(swap_chain.image_count());

		auto alloc_info = VkCommandBufferAllocateInfo{
			.sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = device.get_command_pool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
		};

		if (vkAllocateCommandBuffers(device.get_device(), &alloc_info, command_buffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers");
		}

		for (int i = 0; i < command_buffers.size(); i++) {
			auto begin_info = VkCommandBufferBeginInfo{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
			};

			if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer");
			}

			auto render_pass_info = VkRenderPassBeginInfo{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass = swap_chain.get_render_pass(),
				.framebuffer = swap_chain.get_frame_buffer(i),
				.renderArea{
					.offset = {0, 0},
					.extent = swap_chain.get_swap_chain_extent()
				}
			};

			auto clear_values = std::array<VkClearValue, 2>{
				VkClearValue{
					.color = {0.1f, 0.1f, 0.1f, 1.0f}
				},
				VkClearValue{
					.depthStencil = {1.0f, 0}
				}
			};

			render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

			pipeline->bind(command_buffers[i]);
			vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(command_buffers[i]);
			if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
			  throw std::runtime_error("failed to record command buffer");
			}
		}
	}
	void app::draw_frame()
	{
		uint32_t image_index;
		auto result = swap_chain.acquire_next_image(&image_index);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image");
		}

		result = swap_chain.submit_command_buffers(&command_buffers[image_index], &image_index);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image");
		}
	}
}