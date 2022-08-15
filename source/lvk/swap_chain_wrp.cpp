#include "swap_chain_wrp.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace lvk
{
  swap_chain_wrp::swap_chain_wrp(device_wrp &deviceRef, VkExtent2D extent)
      : device{deviceRef}, window_extent{extent}
  {
    create_swap_chain();
    create_image_views();
    create_render_pass();
    create_depth_resources();
    create_framebuffers();
    create_sync_objects();
  }

  swap_chain_wrp::~swap_chain_wrp()
  {
    for (auto imageView : swap_chain_image_views)
    {
      vkDestroyImageView(device.get_device(), imageView, nullptr);
    }
    swap_chain_image_views.clear();

    if (swap_chain != nullptr)
    {
      vkDestroySwapchainKHR(device.get_device(), swap_chain, nullptr);
      swap_chain = nullptr;
    }

    for (int i = 0; i < depth_images.size(); i++)
    {
      vkDestroyImageView(device.get_device(), depth_image_views[i], nullptr);
      vkDestroyImage(device.get_device(), depth_images[i], nullptr);
      vkFreeMemory(device.get_device(), depth_image_memorys[i], nullptr);
    }

    for (auto framebuffer : swap_chain_framebuffers)
    {
      vkDestroyFramebuffer(device.get_device(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(device.get_device(), render_pass, nullptr);

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(device.get_device(), render_finished_semaphores[i], nullptr);
      vkDestroySemaphore(device.get_device(), image_available_semaphores[i], nullptr);
      vkDestroyFence(device.get_device(), in_flight_fences[i], nullptr);
    }
  }

  VkResult swap_chain_wrp::acquire_next_image(uint32_t *image_index)
  {
    vkWaitForFences(
        device.get_device(),
        1,
        &in_flight_fences[current_frame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
        device.get_device(),
        swap_chain,
        std::numeric_limits<uint64_t>::max(),
        image_available_semaphores[current_frame], // must be a not signaled semaphore
        VK_NULL_HANDLE,
        image_index);

    return result;
  }

  VkResult swap_chain_wrp::submit_command_buffers(
      const VkCommandBuffer *buffers, uint32_t *image_index)
  {
    if (images_in_flight[*image_index] != VK_NULL_HANDLE)
    {
      vkWaitForFences(device.get_device(), 1, &images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
    }
    images_in_flight[*image_index] = in_flight_fences[current_frame];

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {image_available_semaphores[current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {render_finished_semaphores[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.get_device(), 1, &in_flight_fences[current_frame]);
    if (vkQueueSubmit(device.get_graphics_queue(), 1, &submit_info, in_flight_fences[current_frame]) !=
        VK_SUCCESS)
    {
      throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swap_chains[] = {swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;

    present_info.pImageIndices = image_index;

    auto result = vkQueuePresentKHR(device.get_present_queue(), &present_info);

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
  }

  void swap_chain_wrp::create_swap_chain()
  {
    swap_chain_support_details swap_chain_support = device.get_swap_chain_support();

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 &&
        image_count > swap_chain_support.capabilities.maxImageCount)
    {
      image_count = swap_chain_support.capabilities.maxImageCount;
    }

    auto create_info = VkSwapchainCreateInfoKHR{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = device.get_surface(),
		.minImageCount = image_count,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	};

    queue_family_indices indices = device.find_physical_queue_families();
    uint32_t queueFamilyIndices[] = {indices.graphics_family, indices.present_family};

    if (indices.graphics_family != indices.present_family)
    {
      create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.queueFamilyIndexCount = 0;     // Optional
      create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;

    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device.get_device(), &create_info, nullptr, &swap_chain) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create swap chain!");
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(device.get_device(), swap_chain, &image_count, nullptr);
    swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(device.get_device(), swap_chain, &image_count, swap_chain_images.data());

    swap_chain_image_format = surface_format.format;
    swap_chain_extent = extent;
  }

  void swap_chain_wrp::create_image_views()
  {
    swap_chain_image_views.resize(swap_chain_images.size());
    for (size_t i = 0; i < swap_chain_images.size(); i++)
    {
      auto viewInfo = VkImageViewCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		  .image = swap_chain_images[i],
		  .viewType = VK_IMAGE_VIEW_TYPE_2D,
		  .format = swap_chain_image_format,
		  .subresourceRange{
			  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			  .baseMipLevel = 0,
			  .levelCount = 1,
			  .baseArrayLayer = 0,
			  .layerCount = 1,
		  }
	  };


      if (vkCreateImageView(device.get_device(), &viewInfo, nullptr, &swap_chain_image_views[i]) !=
          VK_SUCCESS)
      {
        throw std::runtime_error("failed to create texture image view!");
      }
    }
  }

  void swap_chain_wrp::create_render_pass()
  {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = find_depth_format();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = get_swap_chain_image_format();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstSubpass = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.get_device(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create render pass!");
    }
  }

  void swap_chain_wrp::create_framebuffers()
  {
    swap_chain_framebuffers.resize(image_count());
    for (size_t i = 0; i < image_count(); i++)
    {
      std::array<VkImageView, 2> attachments = {swap_chain_image_views[i], depth_image_views[i]};

      VkExtent2D swap_chain_extent = get_swap_chain_extent();
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = render_pass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swap_chain_extent.width;
      framebufferInfo.height = swap_chain_extent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer(
              device.get_device(),
              &framebufferInfo,
              nullptr,
              &swap_chain_framebuffers[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create framebuffer!");
      }
    }
  }

  void swap_chain_wrp::create_depth_resources()
  {
    VkFormat depthFormat = find_depth_format();
    VkExtent2D swapChainExtent = get_swap_chain_extent();

    depth_images.resize(image_count());
    depth_image_memorys.resize(image_count());
    depth_image_views.resize(image_count());

    for (int i = 0; i < depth_images.size(); i++)
    {
      VkImageCreateInfo imageInfo{};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent.width = swapChainExtent.width;
      imageInfo.extent.height = swapChainExtent.height;
      imageInfo.extent.depth = 1;
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = depthFormat;
      imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageInfo.flags = 0;

      device.createImageWithInfo(
          imageInfo,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
          depth_images[i],
          depth_image_memorys[i]);

      VkImageViewCreateInfo viewInfo{};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = depth_images[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = depthFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView(device.get_device(), &viewInfo, nullptr, &depth_image_views[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create texture image view!");
      }
    }
  }

  void swap_chain_wrp::create_sync_objects()
  {
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    images_in_flight.resize(image_count(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if (vkCreateSemaphore(device.get_device(), &semaphore_info, nullptr, &image_available_semaphores[i]) !=
              VK_SUCCESS ||
          vkCreateSemaphore(device.get_device(), &semaphore_info, nullptr, &render_finished_semaphores[i]) !=
              VK_SUCCESS ||
          vkCreateFence(device.get_device(), &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
      }
    }
  }

  VkSurfaceFormatKHR swap_chain_wrp::choose_swap_surface_format(
      const std::vector<VkSurfaceFormatKHR> &available_formats)
  {
    for (const auto &available_format : available_formats)
    {
      if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
          available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
        return available_format;
      }
    }

    return available_formats[0];
  }

  VkPresentModeKHR swap_chain_wrp::choose_swap_present_mode(
      const std::vector<VkPresentModeKHR> &available_present_modes)
  {
//    for (const auto &available_present_mode : available_present_modes)
//    {
//      if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
//      {
//        std::cout << "Present mode: Mailbox" << std::endl;
//        return available_present_mode;
//      }
//    }

    // for (const auto &available_present_mode : available_present_modes) {
    //   if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
    //     std::cout << "Present mode: Immediate" << std::endl;
    //     return available_present_mode;
    //   }
    // }

    std::cout << "Present mode: V-Sync" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D swap_chain_wrp::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities)
  {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
      return capabilities.currentExtent;
    }
    else
    {
      VkExtent2D actual_extent = window_extent;
      actual_extent.width = std::max(
          capabilities.minImageExtent.width,
          std::min(capabilities.maxImageExtent.width, actual_extent.width));
      actual_extent.height = std::max(
          capabilities.minImageExtent.height,
          std::min(capabilities.maxImageExtent.height, actual_extent.height));

      return actual_extent;
    }
  }

  VkFormat swap_chain_wrp::find_depth_format()
  {
    return device.find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }
}
