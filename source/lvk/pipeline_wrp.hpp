#pragma once

#include "device_wrp.hpp"

#include <string>
#include <vector>

namespace lvk
{
	struct pipeline_config_info
	{

	};

	class pipeline_wrp
	{
		device_wrp& device;
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
		pipeline_wrp(
			device_wrp& _device,
			const pipeline_config_info& _config,
			const std::string& _vert_path,
			const std::string& _frag_path
		);
		~pipeline_wrp()
		{

		};

		pipeline_wrp(const pipeline_wrp&) = delete;
		pipeline_wrp& operator=(const pipeline_wrp&) = delete;

		static auto default_pipeline_config_info(uint32_t width, uint32_t height) -> pipeline_config_info;

	};

}
