#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <vector>

#include <backends/imgui_impl_vulkan.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        rendering_vulkan_context g_vulkan = {};

        [[nodiscard]] bool _imgui_vulkan_renderer_ready()
        {
            return ImGui::GetCurrentContext() != nullptr
                && ImGui::GetIO().BackendRendererUserData != nullptr;
        }

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
        [[nodiscard]] GLADapiproc _glfw_vulkan_load(const char* name)
        {
            return reinterpret_cast<GLADapiproc>(glfwGetInstanceProcAddress(g_vulkan.instance, name));
        }

        void _load_vulkan_functions(const VkPhysicalDevice physical_device)
        {
            if (gladLoadVulkan(physical_device, _glfw_vulkan_load) == 0) {
                LUCARIA_DEBUG_ERROR("Failed to load Vulkan functions")
            }
        }
#endif

        [[nodiscard]] bool _queue_supports_present(VkPhysicalDevice device, uint32 family, VkSurfaceKHR surface)
        {
            VkBool32 _supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, family, surface, &_supported);
            return _supported == VK_TRUE;
        }

        [[nodiscard]] uint32 _find_graphics_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            uint32 _count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &_count, nullptr);
            std::vector<VkQueueFamilyProperties> _families(_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &_count, _families.data());
            for (uint32 _index = 0; _index < _count; ++_index) {
                if ((_families[_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && _queue_supports_present(device, _index, surface)) {
                    return _index;
                }
            }
            LUCARIA_DEBUG_ERROR("No Vulkan graphics/present queue family found")
            return 0;
        }

        [[nodiscard]] VkPhysicalDevice _pick_physical_device(VkInstance instance, VkSurfaceKHR surface)
        {
            uint32 _count = 0;
            vkEnumeratePhysicalDevices(instance, &_count, nullptr);
            if (_count == 0) {
                LUCARIA_DEBUG_ERROR("No Vulkan physical device found")
                return VK_NULL_HANDLE;
            }
            std::vector<VkPhysicalDevice> _devices(_count);
            vkEnumeratePhysicalDevices(instance, &_count, _devices.data());
            for (VkPhysicalDevice _device : _devices) {
                uint32 _family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(_device, &_family_count, nullptr);
                for (uint32 _family = 0; _family < _family_count; ++_family) {
                    if (_queue_supports_present(_device, _family, surface)) {
                        return _device;
                    }
                }
            }
            return _devices.front();
        }

        void _create_instance(const std::vector<const char*>& extensions)
        {
            VkApplicationInfo _app = {};
            _app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            _app.pApplicationName = "Lucaria";
            _app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            _app.pEngineName = "Lucaria";
            _app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            _app.apiVersion = VK_API_VERSION_1_3;

            VkInstanceCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            _create.pApplicationInfo = &_app;
            _create.enabledExtensionCount = static_cast<uint32>(extensions.size());
            _create.ppEnabledExtensionNames = extensions.data();
            if (vkCreateInstance(&_create, nullptr, &g_vulkan.instance) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan instance")
            }
        }

        void _create_device()
        {
            g_vulkan.physical_device = _pick_physical_device(g_vulkan.instance, g_vulkan.surface);
            g_vulkan.graphics_queue_family = _find_graphics_queue_family(g_vulkan.physical_device, g_vulkan.surface);

            const float _priority = 1.0f;
            VkDeviceQueueCreateInfo _queue = {};
            _queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            _queue.queueFamilyIndex = g_vulkan.graphics_queue_family;
            _queue.queueCount = 1;
            _queue.pQueuePriorities = &_priority;

            uint32 _extension_count = 0;
            vkEnumerateDeviceExtensionProperties(g_vulkan.physical_device, nullptr, &_extension_count, nullptr);
            std::vector<VkExtensionProperties> _available_extensions(_extension_count);
            vkEnumerateDeviceExtensionProperties(g_vulkan.physical_device, nullptr, &_extension_count, _available_extensions.data());
            auto _has_extension = [&](const char* name) {
                return std::any_of(_available_extensions.begin(), _available_extensions.end(), [&](const VkExtensionProperties& extension) {
                    return std::strcmp(extension.extensionName, name) == 0;
                });
            };
            std::vector<const char*> _extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            if (_has_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)) {
                _extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            }

            VkPhysicalDeviceDynamicRenderingFeatures _dynamic_rendering = {};
            _dynamic_rendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
            _dynamic_rendering.dynamicRendering = VK_TRUE;

            VkDeviceCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            _create.pNext = &_dynamic_rendering;
            _create.queueCreateInfoCount = 1;
            _create.pQueueCreateInfos = &_queue;
            _create.enabledExtensionCount = static_cast<uint32>(_extensions.size());
            _create.ppEnabledExtensionNames = _extensions.data();
            if (vkCreateDevice(g_vulkan.physical_device, &_create, nullptr, &g_vulkan.device) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan device")
            }
            vkGetDeviceQueue(g_vulkan.device, g_vulkan.graphics_queue_family, 0, &g_vulkan.graphics_queue);
        }

        void _create_command_pool()
        {
            VkCommandPoolCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            _create.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            _create.queueFamilyIndex = g_vulkan.graphics_queue_family;
            if (vkCreateCommandPool(g_vulkan.device, &_create, nullptr, &g_vulkan.upload_command_pool) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan upload command pool")
            }

            _create.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            if (vkCreateCommandPool(g_vulkan.device, &_create, nullptr, &g_vulkan.frame_command_pool) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan frame command pool")
            }
        }

        [[nodiscard]] VkSurfaceFormatKHR _choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
        {
            for (const VkSurfaceFormatKHR& _format : formats) {
                if (_format.format == VK_FORMAT_B8G8R8A8_UNORM && _format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return _format;
                }
            }
            return formats.front();
        }

        [[nodiscard]] VkPresentModeKHR _choose_present_mode(const std::vector<VkPresentModeKHR>& modes)
        {
            for (const VkPresentModeKHR _mode : modes) {
                if (_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return _mode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        [[nodiscard]] VkExtent2D _choose_swapchain_extent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32x2 fallback_size)
        {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max()) {
                return capabilities.currentExtent;
            }
            VkExtent2D _extent = { fallback_size.x, fallback_size.y };
            _extent.width = std::clamp(_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            _extent.height = std::clamp(_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return _extent;
        }

        void _destroy_swapchain()
        {
            if (g_vulkan.device == VK_NULL_HANDLE) {
                return;
            }
            if (!g_vulkan.frame_command_buffers.empty()) {
                vkFreeCommandBuffers(g_vulkan.device, g_vulkan.frame_command_pool, static_cast<uint32>(g_vulkan.frame_command_buffers.size()), g_vulkan.frame_command_buffers.data());
                g_vulkan.frame_command_buffers.clear();
            }
            for (VkImageView& _view : g_vulkan.swapchain_image_views) {
                if (_view != VK_NULL_HANDLE) {
                    vkDestroyImageView(g_vulkan.device, _view, nullptr);
                    _view = VK_NULL_HANDLE;
                }
            }
            g_vulkan.swapchain_image_views.clear();
            g_vulkan.swapchain_images.clear();
            g_vulkan.swapchain_image_layouts.clear();
            if (g_vulkan.swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(g_vulkan.device, g_vulkan.swapchain, nullptr);
                g_vulkan.swapchain = VK_NULL_HANDLE;
            }
        }

        void _create_swapchain(const uint32x2 fallback_size)
        {
            VkSurfaceCapabilitiesKHR _capabilities = {};
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkan.physical_device, g_vulkan.surface, &_capabilities);

            uint32 _format_count = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan.physical_device, g_vulkan.surface, &_format_count, nullptr);
            std::vector<VkSurfaceFormatKHR> _formats(_format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(g_vulkan.physical_device, g_vulkan.surface, &_format_count, _formats.data());

            uint32 _present_mode_count = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkan.physical_device, g_vulkan.surface, &_present_mode_count, nullptr);
            std::vector<VkPresentModeKHR> _present_modes(_present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(g_vulkan.physical_device, g_vulkan.surface, &_present_mode_count, _present_modes.data());

            const VkSurfaceFormatKHR _surface_format = _choose_surface_format(_formats);
            const VkPresentModeKHR _present_mode = _choose_present_mode(_present_modes);
            const VkExtent2D _extent = _choose_swapchain_extent(_capabilities, fallback_size);

            uint32 _image_count = _capabilities.minImageCount + 1;
            if (_capabilities.maxImageCount > 0 && _image_count > _capabilities.maxImageCount) {
                _image_count = _capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR _create = {};
            _create.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            _create.surface = g_vulkan.surface;
            _create.minImageCount = _image_count;
            _create.imageFormat = _surface_format.format;
            _create.imageColorSpace = _surface_format.colorSpace;
            _create.imageExtent = _extent;
            _create.imageArrayLayers = 1;
            _create.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            _create.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _create.preTransform = _capabilities.currentTransform;
            _create.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            _create.presentMode = _present_mode;
            _create.clipped = VK_TRUE;
            if (vkCreateSwapchainKHR(g_vulkan.device, &_create, nullptr, &g_vulkan.swapchain) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan swapchain")
                return;
            }

            g_vulkan.swapchain_format = _surface_format.format;
            g_vulkan.swapchain_extent = _extent;

            uint32 _swapchain_image_count = 0;
            vkGetSwapchainImagesKHR(g_vulkan.device, g_vulkan.swapchain, &_swapchain_image_count, nullptr);
            g_vulkan.swapchain_images.resize(_swapchain_image_count);
            vkGetSwapchainImagesKHR(g_vulkan.device, g_vulkan.swapchain, &_swapchain_image_count, g_vulkan.swapchain_images.data());
            g_vulkan.swapchain_image_layouts.assign(_swapchain_image_count, VK_IMAGE_LAYOUT_UNDEFINED);

            g_vulkan.swapchain_image_views.resize(_swapchain_image_count);
            for (uint32 _index = 0; _index < _swapchain_image_count; ++_index) {
                VkImageViewCreateInfo _view = {};
                _view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                _view.image = g_vulkan.swapchain_images[_index];
                _view.viewType = VK_IMAGE_VIEW_TYPE_2D;
                _view.format = g_vulkan.swapchain_format;
                _view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                _view.subresourceRange.baseMipLevel = 0;
                _view.subresourceRange.levelCount = 1;
                _view.subresourceRange.baseArrayLayer = 0;
                _view.subresourceRange.layerCount = 1;
                if (vkCreateImageView(g_vulkan.device, &_view, nullptr, &g_vulkan.swapchain_image_views[_index]) != VK_SUCCESS) {
                    LUCARIA_DEBUG_ERROR("Failed to create Vulkan swapchain image view")
                }
            }

            g_vulkan.frame_command_buffers.resize(_swapchain_image_count);
            VkCommandBufferAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            _allocate.commandPool = g_vulkan.frame_command_pool;
            _allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            _allocate.commandBufferCount = _swapchain_image_count;
            if (vkAllocateCommandBuffers(g_vulkan.device, &_allocate, g_vulkan.frame_command_buffers.data()) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan frame command buffers")
            }
        }

        void _recreate_swapchain(const uint32x2 size)
        {
            vkDeviceWaitIdle(g_vulkan.device);
            _destroy_swapchain();
            _create_swapchain(size);
        }

        void _create_frame_sync()
        {
            VkSemaphoreCreateInfo _semaphore = {};
            _semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(g_vulkan.device, &_semaphore, nullptr, &g_vulkan.image_available);
            vkCreateSemaphore(g_vulkan.device, &_semaphore, nullptr, &g_vulkan.render_finished);

            VkFenceCreateInfo _fence = {};
            _fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            _fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(g_vulkan.device, &_fence, nullptr, &g_vulkan.frame_fence);
        }

        void _create_descriptor_pool()
        {
            std::array<VkDescriptorPoolSize, 2> _sizes = {
                VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
                VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
            };
            VkDescriptorPoolCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            _create.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            _create.maxSets = 1024;
            _create.poolSizeCount = static_cast<uint32>(_sizes.size());
            _create.pPoolSizes = _sizes.data();
            if (vkCreateDescriptorPool(g_vulkan.device, &_create, nullptr, &g_vulkan.descriptor_pool) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan descriptor pool")
            }
        }

        void _create_render_pass()
        {
            VkAttachmentDescription _color = {};
            _color.format = g_vulkan.swapchain_format;
            _color.samples = VK_SAMPLE_COUNT_1_BIT;
            _color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            _color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            _color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            _color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            _color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            _color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference _color_ref = {};
            _color_ref.attachment = 0;
            _color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription _subpass = {};
            _subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            _subpass.colorAttachmentCount = 1;
            _subpass.pColorAttachments = &_color_ref;

            VkRenderPassCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            _create.attachmentCount = 1;
            _create.pAttachments = &_color;
            _create.subpassCount = 1;
            _create.pSubpasses = &_subpass;
            if (vkCreateRenderPass(g_vulkan.device, &_create, nullptr, &g_vulkan.render_pass) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan render pass")
            }
        }

        void _cmd_transition_image_layout(VkCommandBuffer command_buffer, VkImage image, VkImageAspectFlags aspect, VkImageLayout old_layout, VkImageLayout new_layout)
        {
            if (command_buffer == VK_NULL_HANDLE || image == VK_NULL_HANDLE || old_layout == new_layout) {
                return;
            }

            VkImageMemoryBarrier _barrier = {};
            _barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            _barrier.oldLayout = old_layout;
            _barrier.newLayout = new_layout;
            _barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            _barrier.image = image;
            _barrier.subresourceRange.aspectMask = aspect;
            _barrier.subresourceRange.baseMipLevel = 0;
            _barrier.subresourceRange.levelCount = 1;
            _barrier.subresourceRange.baseArrayLayer = 0;
            _barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags _source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            VkPipelineStageFlags _destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            if (new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                _barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                _barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                _destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                _barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                _barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                _source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                _destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                _barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                _source_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                _destination_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }

            vkCmdPipelineBarrier(command_buffer, _source_stage, _destination_stage, 0, 0, nullptr, 0, nullptr, 1, &_barrier);
        }

        void _finish_current_target()
        {
            if (g_vulkan.rendering_active) {
                vkCmdEndRendering(g_vulkan.current_command_buffer);
                g_vulkan.rendering_active = false;
            }
            if (!g_vulkan.current_target_is_swapchain && g_vulkan.current_color_layout != nullptr && g_vulkan.current_color_image != VK_NULL_HANDLE) {
                _cmd_transition_image_layout(g_vulkan.current_command_buffer, g_vulkan.current_color_image, VK_IMAGE_ASPECT_COLOR_BIT, *g_vulkan.current_color_layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                *g_vulkan.current_color_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

    }

    rendering_vulkan_context& rendering_vulkan()
    {
        return g_vulkan;
    }

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
    void rendering_vulkan_initialize(GLFWwindow* window)
    {
        if (g_vulkan.initialized) {
            return;
        }
        if (!glfwVulkanSupported()) {
            LUCARIA_DEBUG_ERROR("GLFW reports Vulkan is unavailable")
        }
        _load_vulkan_functions(VK_NULL_HANDLE);
        uint32 _extension_count = 0;
        const char** _extensions = glfwGetRequiredInstanceExtensions(&_extension_count);
        std::vector<const char*> _instance_extensions(_extensions, _extensions + _extension_count);
        _create_instance(_instance_extensions);
        _load_vulkan_functions(VK_NULL_HANDLE);
        if (glfwCreateWindowSurface(g_vulkan.instance, window, nullptr, &g_vulkan.surface) != VK_SUCCESS) {
            LUCARIA_DEBUG_ERROR("Failed to create GLFW Vulkan surface")
        }
        _create_device();
        _load_vulkan_functions(g_vulkan.physical_device);
        _create_command_pool();
        int _width = 1;
        int _height = 1;
        glfwGetFramebufferSize(window, &_width, &_height);
        _create_swapchain({ static_cast<uint32>(std::max(_width, 1)), static_cast<uint32>(std::max(_height, 1)) });
        _create_render_pass();
        _create_descriptor_pool();
        _create_frame_sync();
        g_vulkan.initialized = true;
    }
#endif

#if defined(LUCARIA_PLATFORM_ANDROID)
    void rendering_vulkan_initialize(ANativeWindow* window)
    {
        if (g_vulkan.initialized) {
            return;
        }
        const std::vector<const char*> _instance_extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        };
        _create_instance(_instance_extensions);

        VkAndroidSurfaceCreateInfoKHR _surface = {};
        _surface.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        _surface.window = window;
        if (vkCreateAndroidSurfaceKHR(g_vulkan.instance, &_surface, nullptr, &g_vulkan.surface) != VK_SUCCESS) {
            LUCARIA_DEBUG_ERROR("Failed to create Android Vulkan surface")
        }
        _create_device();
        _create_command_pool();
        _create_swapchain({ 1, 1 });
        _create_render_pass();
        _create_descriptor_pool();
        _create_frame_sync();
        g_vulkan.initialized = true;
    }
#endif

    void rendering_vulkan_shutdown()
    {
        if (!g_vulkan.initialized) {
            return;
        }
        vkDeviceWaitIdle(g_vulkan.device);
        _destroy_swapchain();
        if (g_vulkan.image_available != VK_NULL_HANDLE) {
            vkDestroySemaphore(g_vulkan.device, g_vulkan.image_available, nullptr);
        }
        if (g_vulkan.render_finished != VK_NULL_HANDLE) {
            vkDestroySemaphore(g_vulkan.device, g_vulkan.render_finished, nullptr);
        }
        if (g_vulkan.frame_fence != VK_NULL_HANDLE) {
            vkDestroyFence(g_vulkan.device, g_vulkan.frame_fence, nullptr);
        }
        if (g_vulkan.descriptor_pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(g_vulkan.device, g_vulkan.descriptor_pool, nullptr);
        }
        if (g_vulkan.render_pass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(g_vulkan.device, g_vulkan.render_pass, nullptr);
        }
        if (g_vulkan.upload_command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(g_vulkan.device, g_vulkan.upload_command_pool, nullptr);
        }
        if (g_vulkan.frame_command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(g_vulkan.device, g_vulkan.frame_command_pool, nullptr);
        }
        if (g_vulkan.device != VK_NULL_HANDLE) {
            vkDestroyDevice(g_vulkan.device, nullptr);
        }
        if (g_vulkan.surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(g_vulkan.instance, g_vulkan.surface, nullptr);
        }
        if (g_vulkan.instance != VK_NULL_HANDLE) {
            vkDestroyInstance(g_vulkan.instance, nullptr);
        }
        g_vulkan = {};
    }

    void rendering_vulkan_begin_frame(const uint32x2 size)
    {
        if (!g_vulkan.initialized || size == uint32x2(0)) {
            return;
        }
        if (g_vulkan.swapchain_extent.width != size.x || g_vulkan.swapchain_extent.height != size.y) {
            _recreate_swapchain(size);
        }

        vkWaitForFences(g_vulkan.device, 1, &g_vulkan.frame_fence, VK_TRUE, UINT64_MAX);

        const VkResult _acquire = vkAcquireNextImageKHR(g_vulkan.device, g_vulkan.swapchain, UINT64_MAX, g_vulkan.image_available, VK_NULL_HANDLE, &g_vulkan.current_image);
        if (_acquire == VK_ERROR_OUT_OF_DATE_KHR) {
            _recreate_swapchain(size);
            return;
        }
        vkResetFences(g_vulkan.device, 1, &g_vulkan.frame_fence);

        g_vulkan.current_command_buffer = g_vulkan.frame_command_buffers[g_vulkan.current_image];
        vkResetCommandBuffer(g_vulkan.current_command_buffer, 0);

        VkCommandBufferBeginInfo _begin = {};
        _begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        _begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(g_vulkan.current_command_buffer, &_begin);

        g_vulkan.frame_active = true;
        g_vulkan.rendering_active = false;
        rendering_vulkan_use_default_target();
    }

    void rendering_vulkan_end_frame()
    {
        if (!g_vulkan.frame_active) {
            return;
        }
        _finish_current_target();
        if (!g_vulkan.current_target_is_swapchain) {
            rendering_vulkan_use_default_target();
        }
        if (g_vulkan.current_color_layout != nullptr && g_vulkan.current_color_image != VK_NULL_HANDLE) {
            _cmd_transition_image_layout(g_vulkan.current_command_buffer, g_vulkan.current_color_image, VK_IMAGE_ASPECT_COLOR_BIT, *g_vulkan.current_color_layout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            *g_vulkan.current_color_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        vkEndCommandBuffer(g_vulkan.current_command_buffer);

        const VkPipelineStageFlags _wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo _submit = {};
        _submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        _submit.waitSemaphoreCount = 1;
        _submit.pWaitSemaphores = &g_vulkan.image_available;
        _submit.pWaitDstStageMask = &_wait_stage;
        _submit.commandBufferCount = 1;
        _submit.pCommandBuffers = &g_vulkan.current_command_buffer;
        _submit.signalSemaphoreCount = 1;
        _submit.pSignalSemaphores = &g_vulkan.render_finished;
        vkQueueSubmit(g_vulkan.graphics_queue, 1, &_submit, g_vulkan.frame_fence);

        VkPresentInfoKHR _present = {};
        _present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        _present.waitSemaphoreCount = 1;
        _present.pWaitSemaphores = &g_vulkan.render_finished;
        _present.swapchainCount = 1;
        _present.pSwapchains = &g_vulkan.swapchain;
        _present.pImageIndices = &g_vulkan.current_image;
        vkQueuePresentKHR(g_vulkan.graphics_queue, &_present);

        g_vulkan.current_command_buffer = VK_NULL_HANDLE;
        g_vulkan.frame_active = false;
        g_vulkan.rendering_active = false;
    }

    void rendering_vulkan_initialize_imgui()
    {
        if (ImGui::GetCurrentContext() == nullptr || _imgui_vulkan_renderer_ready()) {
            return;
        }
        ImGui_ImplVulkan_InitInfo _info = {};
        _info.Instance = g_vulkan.instance;
        _info.PhysicalDevice = g_vulkan.physical_device;
        _info.Device = g_vulkan.device;
        _info.QueueFamily = g_vulkan.graphics_queue_family;
        _info.Queue = g_vulkan.graphics_queue;
        _info.DescriptorPool = g_vulkan.descriptor_pool;
        _info.MinImageCount = 2;
        _info.ImageCount = static_cast<uint32>(g_vulkan.swapchain_images.size());
        _info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        _info.UseDynamicRendering = true;
        _info.ColorAttachmentFormat = g_vulkan.swapchain_format;
        ImGui_ImplVulkan_Init(&_info, VK_NULL_HANDLE);
    }

    void rendering_vulkan_shutdown_imgui()
    {
        if (_imgui_vulkan_renderer_ready()) {
            ImGui_ImplVulkan_Shutdown();
        }
    }

    void rendering_vulkan_render_imgui(ImDrawData* draw_data)
    {
        if (draw_data == nullptr || !g_vulkan.frame_active) {
            return;
        }
        rendering_vulkan_begin_rendering(false, false);
        ImGui_ImplVulkan_RenderDrawData(draw_data, g_vulkan.current_command_buffer);
    }

    void rendering_vulkan_use_default_target()
    {
        if (!g_vulkan.frame_active || g_vulkan.current_image >= g_vulkan.swapchain_image_views.size()) {
            return;
        }
        _finish_current_target();
        g_vulkan.current_color_image = g_vulkan.swapchain_images[g_vulkan.current_image];
        g_vulkan.current_color_view = g_vulkan.swapchain_image_views[g_vulkan.current_image];
        g_vulkan.current_color_layout = &g_vulkan.swapchain_image_layouts[g_vulkan.current_image];
        g_vulkan.current_color_format = g_vulkan.swapchain_format;
        g_vulkan.current_depth_image = VK_NULL_HANDLE;
        g_vulkan.current_depth_view = VK_NULL_HANDLE;
        g_vulkan.current_depth_layout = nullptr;
        g_vulkan.current_depth_format = VK_FORMAT_UNDEFINED;
        g_vulkan.current_target_size = { g_vulkan.swapchain_extent.width, g_vulkan.swapchain_extent.height };
        g_vulkan.current_target_is_swapchain = true;
    }

    void rendering_vulkan_use_target(const VkImage color_image, const VkImageView color_view, const VkFormat color_format, VkImageLayout* color_layout, const uint32x2 size, const VkImage depth_image, const VkImageView depth_view, const VkFormat depth_format, VkImageLayout* depth_layout)
    {
        if (!g_vulkan.frame_active || color_image == VK_NULL_HANDLE || color_view == VK_NULL_HANDLE || color_layout == nullptr) {
            return;
        }
        _finish_current_target();
        g_vulkan.current_color_image = color_image;
        g_vulkan.current_color_view = color_view;
        g_vulkan.current_color_layout = color_layout;
        g_vulkan.current_color_format = color_format;
        g_vulkan.current_depth_image = depth_image;
        g_vulkan.current_depth_view = depth_view;
        g_vulkan.current_depth_layout = depth_layout;
        g_vulkan.current_depth_format = depth_format;
        g_vulkan.current_target_size = size;
        g_vulkan.current_target_is_swapchain = false;
    }

    void rendering_vulkan_begin_rendering(const bool clear_color, const bool clear_depth)
    {
        if (!g_vulkan.frame_active || g_vulkan.rendering_active || g_vulkan.current_color_view == VK_NULL_HANDLE || g_vulkan.current_color_layout == nullptr) {
            return;
        }
        _cmd_transition_image_layout(g_vulkan.current_command_buffer, g_vulkan.current_color_image, VK_IMAGE_ASPECT_COLOR_BIT, *g_vulkan.current_color_layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        *g_vulkan.current_color_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkRenderingAttachmentInfo _color = {};
        _color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        _color.imageView = g_vulkan.current_color_view;
        _color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        _color.loadOp = clear_color ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        _color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        _color.clearValue.color = { { 1.f, 1.f, 1.f, 1.f } };

        VkRenderingAttachmentInfo _depth = {};
        if (g_vulkan.current_depth_view != VK_NULL_HANDLE && g_vulkan.current_depth_layout != nullptr) {
            _cmd_transition_image_layout(g_vulkan.current_command_buffer, g_vulkan.current_depth_image, VK_IMAGE_ASPECT_DEPTH_BIT, *g_vulkan.current_depth_layout, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            *g_vulkan.current_depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            _depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            _depth.imageView = g_vulkan.current_depth_view;
            _depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            _depth.loadOp = clear_depth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
            _depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            _depth.clearValue.depthStencil = { 0.f, 0 };
        }

        VkRenderingInfo _rendering = {};
        _rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        _rendering.renderArea.offset = { 0, 0 };
        _rendering.renderArea.extent = { g_vulkan.current_target_size.x, g_vulkan.current_target_size.y };
        _rendering.layerCount = 1;
        _rendering.colorAttachmentCount = 1;
        _rendering.pColorAttachments = &_color;
        _rendering.pDepthAttachment = _depth.imageView != VK_NULL_HANDLE ? &_depth : nullptr;
        vkCmdBeginRendering(g_vulkan.current_command_buffer, &_rendering);
        g_vulkan.rendering_active = true;
    }

    void rendering_vulkan_end_rendering()
    {
        if (g_vulkan.rendering_active) {
            vkCmdEndRendering(g_vulkan.current_command_buffer);
            g_vulkan.rendering_active = false;
        }
    }

    VkCommandBuffer rendering_vulkan_command_buffer()
    {
        return g_vulkan.current_command_buffer;
    }

    VkFormat rendering_vulkan_current_color_format()
    {
        return g_vulkan.current_color_format;
    }

    VkFormat rendering_vulkan_current_depth_format()
    {
        return g_vulkan.current_depth_format;
    }

    uint32x2 rendering_vulkan_current_target_size()
    {
        return g_vulkan.current_target_size;
    }

    uint32 rendering_vulkan_find_memory_type(const uint32 type_filter, const VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties _memory_properties = {};
        vkGetPhysicalDeviceMemoryProperties(g_vulkan.physical_device, &_memory_properties);
        for (uint32 _index = 0; _index < _memory_properties.memoryTypeCount; ++_index) {
            const bool _type_matches = (type_filter & (1u << _index)) != 0;
            const bool _properties_match = (_memory_properties.memoryTypes[_index].propertyFlags & properties) == properties;
            if (_type_matches && _properties_match) {
                return _index;
            }
        }
        LUCARIA_DEBUG_ERROR("Failed to find Vulkan memory type")
        return 0;
    }

    VkCommandBuffer rendering_vulkan_begin_upload_commands()
    {
        VkCommandBufferAllocateInfo _allocate = {};
        _allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        _allocate.commandPool = g_vulkan.upload_command_pool;
        _allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        _allocate.commandBufferCount = 1;

        VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
        if (vkAllocateCommandBuffers(g_vulkan.device, &_allocate, &_command_buffer) != VK_SUCCESS) {
            LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan upload command buffer")
            return VK_NULL_HANDLE;
        }

        VkCommandBufferBeginInfo _begin = {};
        _begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        _begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(_command_buffer, &_begin);
        return _command_buffer;
    }

    void rendering_vulkan_end_upload_commands(VkCommandBuffer command_buffer)
    {
        if (command_buffer == VK_NULL_HANDLE) {
            return;
        }
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo _submit = {};
        _submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        _submit.commandBufferCount = 1;
        _submit.pCommandBuffers = &command_buffer;
        vkQueueSubmit(g_vulkan.graphics_queue, 1, &_submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(g_vulkan.graphics_queue);
        vkFreeCommandBuffers(g_vulkan.device, g_vulkan.upload_command_pool, 1, &command_buffer);
    }

}
}

#endif
