#include "pipeline_wrp.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lvk
{
	pipeline_wrp::pipeline_wrp(
		device_wrp& _device,
		const pipeline_config_info& _config,
		const std::string& _vert_path,
		const std::string& _frag_path
	) : device{ _device }
	{
		create_graphics_pipeline(_config, _vert_path, _frag_path);

	}

	void pipeline_wrp::create_graphics_pipeline(
		const pipeline_config_info& config,
		const std::string& vert_path,
		const std::string& frag_path)
	{
		auto vert_data = read_file(vert_path);
		auto frag_data = read_file(frag_path);

		std::cout << "vert size: " << vert_data.size() << '\n'
				  << "frag size: " << frag_data.size() << '\n';

	}

	auto pipeline_wrp::read_file(const std::string& file_path) -> std::vector<char>
	{
		std::ifstream file{ file_path, std::ios::ate | std::ios::binary };
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file: " + file_path);
		}

		auto file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		file.close();

		return buffer;
	}

	void pipeline_wrp::create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module)
	{
		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(device.get_device(), &create_info, nullptr, shader_module) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}
	}

	auto pipeline_wrp::default_pipeline_config_info(uint32_t width, uint32_t height) -> pipeline_config_info
	{
		pipeline_config_info config_info{};

		return config_info;
	}

}