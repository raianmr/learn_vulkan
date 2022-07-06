#pragma once

#include "apparatus.hpp"

#include <string>
#include <vector>

namespace lvk
{
	struct pipeline_config_info
	{

	};
	class channel
	{
		apparatus& device;
		VkPipeline graphics_pipeline;
		VkShaderModule vert_shader_module, frag_shader_module;

		static auto read_file(const std::string& file_path) -> std::vector<char>;

		void create_graphics_pipeline(
			const pipeline_config_info& config,
			const std::string& vert_path,
			const std::string& frag_path
		);

		void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);

	public:
		channel(
			apparatus& device,
			const pipeline_config_info& config,
			const std::string& vert_path,
			const std::string& frag_path
		);
		~channel()
		{

		};

		channel(const channel&) = delete;
		channel& operator=(const channel&) = delete;

		static auto default_pipeline_config_info(uint32_t width, uint32_t height) -> pipeline_config_info;

	};

}
