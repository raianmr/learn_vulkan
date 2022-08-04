#pragma once

#include "device_wrp.hpp"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace lvk
{
  class swap_chain_wrp
  {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    swap_chain_wrp(device_wrp &device_ref, VkExtent2D window_extent);
    ~swap_chain_wrp();

    swap_chain_wrp(const swap_chain_wrp &) = delete;
    void operator=(const swap_chain_wrp &) = delete;

    VkFramebuffer get_frame_buffer(int index) { return swap_chain_framebuffers[index]; }
    VkRenderPass get_render_pass() { return render_pass; }
    VkImageView get_image_view(int index) { return swap_chain_image_views[index]; }
    size_t image_count() { return swap_chain_images.size(); }
    VkFormat get_swap_chain_image_format() { return swap_chain_image_format; }
    VkExtent2D get_swap_chain_extent() { return swap_chain_extent; }
    uint32_t width() { return swap_chain_extent.width; }
    uint32_t height() { return swap_chain_extent.height; }

    float extent_aspect_ratio()
    {
      return static_cast<float>(swap_chain_extent.width) / static_cast<float>(swap_chain_extent.height);
    }
    VkFormat find_depth_format();

    VkResult acquire_next_image(uint32_t *image_index);
    VkResult submit_command_buffers(const VkCommandBuffer *buffers, uint32_t *image_index);

  private:
    void create_swap_chain();
    void create_image_views();
    void create_depth_resources();
    void create_render_pass();
    void create_framebuffers();
    void create_sync_objects();

    // Helper functions
    VkSurfaceFormatKHR choose_swap_surface_format(
        const std::vector<VkSurfaceFormatKHR> &available_formats);
    VkPresentModeKHR choose_swap_present_mode(
        const std::vector<VkPresentModeKHR> &available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;

    std::vector<VkFramebuffer> swap_chain_framebuffers;
    VkRenderPass render_pass;

    std::vector<VkImage> depth_images;
    std::vector<VkDeviceMemory> depth_image_memorys;
    std::vector<VkImageView> depth_image_views;
    std::vector<VkImage> swap_chain_images;
    std::vector<VkImageView> swap_chain_image_views;

    device_wrp &device;
    VkExtent2D window_extent;

    VkSwapchainKHR swap_chain;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkSemaphore> render_finished_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<VkFence> images_in_flight;
    size_t current_frame = 0;
  };
}
