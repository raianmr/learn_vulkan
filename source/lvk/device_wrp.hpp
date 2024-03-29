#pragma once

#include "window_wrp.hpp"

#include <string>
#include <vector>

namespace lvk
{

	struct swap_chain_support_details
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	struct queue_family_indices
	{
		uint32_t graphics_family;
		uint32_t present_family;
		bool graphics_family_has_value = false;
		bool present_family_has_value = false;
		bool is_complete()
		{
			return graphics_family_has_value && present_family_has_value;
		}
	};

	class device_wrp
	{
	public:
#ifdef NDEBUG
		const bool enable_validation_layers = false;
#else
		const bool enable_validation_layers = true;
#endif

		explicit device_wrp(window_wrp &_window);
		~device_wrp();

		// Not copyable or movable
		device_wrp(const device_wrp &) = delete;
		void operator=(const device_wrp &) = delete;
		device_wrp(device_wrp &&) = delete;
		device_wrp &operator=(device_wrp &&) = delete;

		VkCommandPool get_command_pool()
		{
			return command_pool;
		}
		VkDevice get_device()
		{
			return device;
		}
		VkSurfaceKHR get_surface()
		{
			return surface;
		}
		VkQueue get_graphics_queue()
		{
			return graphics_queue;
		}
		VkQueue get_present_queue()
		{
			return present_queue;
		}

		swap_chain_support_details get_swap_chain_support()
		{
			return query_swap_chain_support(physical_device);
		}
		uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		queue_family_indices find_physical_queue_families()
		{
			return find_queue_families(physical_device);
		}
		VkFormat find_supported_format(
			const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		// Buffer Helper Functions
		void createBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer &buffer,
			VkDeviceMemory &buffer_memory);
		VkCommandBuffer begin_single_time_commands();
		void end_single_time_commands(VkCommandBuffer command_buffer);
		void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
		void copyBufferToImage(
			VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

		void createImageWithInfo(
			const VkImageCreateInfo &image_info,
			VkMemoryPropertyFlags properties,
			VkImage &image,
			VkDeviceMemory &image_memory);

		VkPhysicalDeviceProperties properties;

	private:
		void create_instance();
		void setup_debug_messenger();
		void create_surface();
		void pick_physical_device();
		void create_logical_device();
		void create_command_pool();

		// helper functions
		bool is_device_suitable(VkPhysicalDevice device);
		std::vector<const char *> get_required_extensions();
		bool check_validation_layer_support();
		queue_family_indices find_queue_families(VkPhysicalDevice device);
		void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
		void has_gflw_required_instance_extensions();
		bool check_device_extension_support(VkPhysicalDevice device);
		swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device);

		VkInstance instance;
		VkDebugUtilsMessengerEXT debug_messenger;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;
		window_wrp &window;
		VkCommandPool command_pool;

		VkDevice device;
		VkSurfaceKHR surface;
		VkQueue graphics_queue;
		VkQueue present_queue;

		const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	};

} // namespace lvk
