#pragma once

#if defined(LUCARIA_BACKEND_VULKAN)

#if defined(LUCARIA_PLATFORM_ANDROID)
#include <android/native_window.h>
#endif

#include <lucaria/core/rendering_backend.hpp>
#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/utils_math.hpp>

#include <vector>

struct ImDrawData;

namespace lucaria {
namespace detail {

    struct rendering_vulkan_context {
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphics_queue = VK_NULL_HANDLE;
        uint32 graphics_queue_family = 0;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkExtent2D swapchain_extent = {};
        std::vector<VkImage> swapchain_images = {};
        std::vector<VkImageView> swapchain_image_views = {};
        std::vector<VkImageLayout> swapchain_image_layouts = {};
        VkCommandPool frame_command_pool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> frame_command_buffers = {};
        VkSemaphore image_available = VK_NULL_HANDLE;
        VkSemaphore render_finished = VK_NULL_HANDLE;
        VkFence frame_fence = VK_NULL_HANDLE;
        uint32 current_image = 0;
        VkCommandBuffer current_command_buffer = VK_NULL_HANDLE;
        VkImage current_color_image = VK_NULL_HANDLE;
        VkImageView current_color_view = VK_NULL_HANDLE;
        VkImageLayout* current_color_layout = nullptr;
        VkFormat current_color_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkImage current_depth_image = VK_NULL_HANDLE;
        VkImageView current_depth_view = VK_NULL_HANDLE;
        VkImageLayout* current_depth_layout = nullptr;
        VkFormat current_depth_format = VK_FORMAT_UNDEFINED;
        uint32x2 current_target_size = {};
        bool current_target_is_swapchain = false;
        bool frame_active = false;
        bool rendering_active = false;
        VkCommandPool upload_command_pool = VK_NULL_HANDLE;
        VkRenderPass render_pass = VK_NULL_HANDLE;
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        bool initialized = false;
    };

    [[nodiscard]] rendering_vulkan_context& rendering_vulkan();

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
    void rendering_vulkan_initialize(GLFWwindow* window);
#endif
#if defined(LUCARIA_PLATFORM_ANDROID)
    void rendering_vulkan_initialize(ANativeWindow* window);
#endif
    void rendering_vulkan_shutdown();
    void rendering_vulkan_begin_frame(const uint32x2 size);
    void rendering_vulkan_end_frame();
    void rendering_vulkan_initialize_imgui();
    void rendering_vulkan_shutdown_imgui();
    void rendering_vulkan_render_imgui(ImDrawData* draw_data);
    void rendering_vulkan_use_default_target();
    void rendering_vulkan_use_target(VkImage color_image, VkImageView color_view, VkFormat color_format, VkImageLayout* color_layout, uint32x2 size, VkImage depth_image = VK_NULL_HANDLE, VkImageView depth_view = VK_NULL_HANDLE, VkFormat depth_format = VK_FORMAT_UNDEFINED, VkImageLayout* depth_layout = nullptr);
    void rendering_vulkan_begin_rendering(bool clear_color, bool clear_depth);
    void rendering_vulkan_end_rendering();
    [[nodiscard]] VkCommandBuffer rendering_vulkan_command_buffer();
    [[nodiscard]] VkFormat rendering_vulkan_current_color_format();
    [[nodiscard]] VkFormat rendering_vulkan_current_depth_format();
    [[nodiscard]] uint32x2 rendering_vulkan_current_target_size();
    [[nodiscard]] uint32 rendering_vulkan_find_memory_type(uint32 type_filter, VkMemoryPropertyFlags properties);
    void rendering_vulkan_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    void rendering_vulkan_destroy_buffer(VkBuffer& buffer, VkDeviceMemory& memory);
    void rendering_vulkan_upload_buffer(VkBuffer destination, VkDeviceSize destination_offset, const void* data, VkDeviceSize size);
    [[nodiscard]] VkFormat rendering_vulkan_image_format(data_image_profile profile);
    void rendering_vulkan_create_image(uint32x2 size, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspect, VkImage& image, VkDeviceMemory& memory, VkImageView& view, VkImageLayout& layout, uint32 layers = 1, VkImageCreateFlags flags = 0);
    void rendering_vulkan_destroy_image(VkImage& image, VkDeviceMemory& memory, VkImageView& view);
    void rendering_vulkan_create_sampler(VkSampler& sampler);
    void rendering_vulkan_destroy_sampler(VkSampler& sampler);
    void rendering_vulkan_upload_image_region(VkImage image, uint32x2 offset, uint32x2 size, const void* data, VkDeviceSize size_bytes, VkImageAspectFlags aspect, VkImageLayout& layout, uint32 layer = 0, uint32 layers = 1);
    void rendering_vulkan_upload_image(VkImage image, uint32x2 size, const void* data, VkDeviceSize size_bytes, VkImageAspectFlags aspect, VkImageLayout& layout, uint32 layer = 0, uint32 layers = 1);
    [[nodiscard]] VkDescriptorSet rendering_vulkan_add_imgui_texture(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout);
    void rendering_vulkan_remove_imgui_texture(VkDescriptorSet& descriptor);

}
}

#endif
