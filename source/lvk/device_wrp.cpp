#include "device_wrp.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace lvk
{

	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void *pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks *pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	// class member functions
	device_wrp::device_wrp(window_wrp &_window) : window{_window}
	{
		create_instance();
		setup_debug_messenger();
		create_surface();
		pick_physical_device();
		create_logical_device();
		create_command_pool();
	}

	device_wrp::~device_wrp()
	{
		vkDestroyCommandPool(device, command_pool, nullptr);
		vkDestroyDevice(device, nullptr);

		if (enable_validation_layers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
	}

	void device_wrp::create_instance()
	{
		if (enable_validation_layers && !check_validation_layer_support())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "LittleVulkanEngine App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = get_required_extensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enable_validation_layers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			createInfo.ppEnabledLayerNames = validation_layers.data();

			populate_debug_messenger_create_info(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		has_gflw_required_instance_extensions();
	}

	void device_wrp::pick_physical_device()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::cout << "Device count: " << deviceCount << std::endl;
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto &device : devices)
		{
			if (is_device_suitable(device))
			{
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}

		vkGetPhysicalDeviceProperties(physical_device, &properties);
		std::cout << "physical get_device: " << properties.deviceName << std::endl;
	}

	void device_wrp::create_logical_device()
	{
		queue_family_indices indices = find_queue_families(physical_device);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family, indices.present_family};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		createInfo.ppEnabledExtensionNames = device_extensions.data();

		// might not really be necessary anymore because get_device specific validation layers
		// have been deprecated
		if (enable_validation_layers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			createInfo.ppEnabledLayerNames = validation_layers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physical_device, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical get_device!");
		}

		vkGetDeviceQueue(device, indices.graphics_family, 0, &graphics_queue);
		vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);
	}

	void device_wrp::create_command_pool()
	{
		queue_family_indices queue_family_indices = find_physical_queue_families();

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queue_family_indices.graphics_family;
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &command_pool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void device_wrp::create_surface()
	{
		window.create_window_surface(instance, &surface);
	}

	bool device_wrp::is_device_suitable(VkPhysicalDevice device)
	{
		queue_family_indices indices = find_queue_families(device);

		bool extensionsSupported = check_device_extension_support(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			swap_chain_support_details swap_chain_support = query_swap_chain_support(device);
			swapChainAdequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.is_complete() && extensionsSupported && swapChainAdequate &&
			   supportedFeatures.samplerAnisotropy;
	}

	void device_wrp::populate_debug_messenger_create_info(
		VkDebugUtilsMessengerCreateInfoEXT &createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	void device_wrp::setup_debug_messenger()
	{
		if (!enable_validation_layers)
			return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populate_debug_messenger_create_info(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debug_messenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	bool device_wrp::check_validation_layer_support()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName : validation_layers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char *> device_wrp::get_required_extensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enable_validation_layers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void device_wrp::has_gflw_required_instance_extensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:" << std::endl;
		std::unordered_set<std::string> available;
		for (const auto &extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert(extension.extensionName);
		}

		std::cout << "required extensions:" << std::endl;
		auto requiredExtensions = get_required_extensions();
		for (const auto &required : requiredExtensions)
		{
			std::cout << "\t" << required << std::endl;
			if (available.find(required) == available.end())
			{
				throw std::runtime_error("Missing required glfw extension");
			}
		}
	}

	bool device_wrp::check_device_extension_support(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
			device,
			nullptr,
			&extensionCount,
			availableExtensions.data());

		std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

		for (const auto &extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	queue_family_indices device_wrp::find_queue_families(VkPhysicalDevice device)
	{
		queue_family_indices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto &queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_family = i;
				indices.graphics_family_has_value = true;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
			{
				indices.present_family = i;
				indices.present_family_has_value = true;
			}
			if (indices.is_complete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	swap_chain_support_details device_wrp::query_swap_chain_support(VkPhysicalDevice device)
	{
		swap_chain_support_details details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.present_modes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				surface,
				&presentModeCount,
				details.present_modes.data());
		}
		return details;
	}

	VkFormat device_wrp::find_supported_format(
		const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (
				tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	uint32_t device_wrp::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void device_wrp::createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer &buffer,
		VkDeviceMemory &buffer_memory)
	{
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device, buffer, buffer_memory, 0);
	}

	VkCommandBuffer device_wrp::begin_single_time_commands()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);
		return command_buffer;
	}

	void device_wrp::end_single_time_commands(VkCommandBuffer command_buffer)
	{
		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
	}

	void device_wrp::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
	{
		VkCommandBuffer command_buffer = begin_single_time_commands();

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0; // Optional
		copy_region.dstOffset = 0; // Optional
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

		end_single_time_commands(command_buffer);
	}

	void device_wrp::copyBufferToImage(
		VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count)
	{
		VkCommandBuffer command_buffer = begin_single_time_commands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layer_count;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {width, height, 1};

		vkCmdCopyBufferToImage(
			command_buffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
		end_single_time_commands(command_buffer);
	}

	void device_wrp::createImageWithInfo(
		const VkImageCreateInfo &image_info,
		VkMemoryPropertyFlags properties,
		VkImage &image,
		VkDeviceMemory &image_memory)
	{
		if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(device, image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		if (vkBindImageMemory(device, image, image_memory, 0) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to bind image memory!");
		}
	}

} // namespace lvk
