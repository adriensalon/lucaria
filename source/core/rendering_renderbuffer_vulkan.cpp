#if defined(LUCARIA_BACKEND_VULKAN)

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_renderbuffer.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        [[nodiscard]] bool _is_depth_format(const VkFormat format)
        {
            switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;
            default:
                return false;
            }
        }

        void _create_renderbuffer_image(rendering_renderbuffer& renderbuffer)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            const VkImageAspectFlags _aspect = _is_depth_format(renderbuffer.internal_format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            const VkImageUsageFlags _usage = _is_depth_format(renderbuffer.internal_format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            VkImageCreateInfo _create = {};
            _create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            _create.imageType = VK_IMAGE_TYPE_2D;
            _create.format = renderbuffer.internal_format;
            _create.extent = { renderbuffer.size.x, renderbuffer.size.y, 1 };
            _create.mipLevels = 1;
            _create.arrayLayers = 1;
            _create.samples = VK_SAMPLE_COUNT_1_BIT;
            _create.tiling = VK_IMAGE_TILING_OPTIMAL;
            _create.usage = _usage;
            _create.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            _create.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (vkCreateImage(_vulkan.device, &_create, nullptr, &renderbuffer.image) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan renderbuffer image")
                return;
            }

            VkMemoryRequirements _requirements = {};
            vkGetImageMemoryRequirements(_vulkan.device, renderbuffer.image, &_requirements);

            VkMemoryAllocateInfo _allocate = {};
            _allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            _allocate.allocationSize = _requirements.size;
            _allocate.memoryTypeIndex = rendering_vulkan_find_memory_type(_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            if (vkAllocateMemory(_vulkan.device, &_allocate, nullptr, &renderbuffer.memory) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to allocate Vulkan renderbuffer image memory")
                return;
            }
            vkBindImageMemory(_vulkan.device, renderbuffer.image, renderbuffer.memory, 0);

            VkImageViewCreateInfo _view = {};
            _view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            _view.image = renderbuffer.image;
            _view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            _view.format = renderbuffer.internal_format;
            _view.subresourceRange.aspectMask = _aspect;
            _view.subresourceRange.baseMipLevel = 0;
            _view.subresourceRange.levelCount = 1;
            _view.subresourceRange.baseArrayLayer = 0;
            _view.subresourceRange.layerCount = 1;
            if (vkCreateImageView(_vulkan.device, &_view, nullptr, &renderbuffer.image_view) != VK_SUCCESS) {
                LUCARIA_DEBUG_ERROR("Failed to create Vulkan renderbuffer image view")
            }
            renderbuffer.layout = VK_IMAGE_LAYOUT_UNDEFINED;
        }

        void _destroy_renderbuffer_image(rendering_renderbuffer& renderbuffer)
        {
            rendering_vulkan_context& _vulkan = rendering_vulkan();
            if (_vulkan.device == VK_NULL_HANDLE) {
                renderbuffer.image = VK_NULL_HANDLE;
                renderbuffer.memory = VK_NULL_HANDLE;
                renderbuffer.image_view = VK_NULL_HANDLE;
                return;
            }
            if (renderbuffer.image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(_vulkan.device, renderbuffer.image_view, nullptr);
                renderbuffer.image_view = VK_NULL_HANDLE;
            }
            if (renderbuffer.image != VK_NULL_HANDLE) {
                vkDestroyImage(_vulkan.device, renderbuffer.image, nullptr);
                renderbuffer.image = VK_NULL_HANDLE;
            }
            if (renderbuffer.memory != VK_NULL_HANDLE) {
                vkFreeMemory(_vulkan.device, renderbuffer.memory, nullptr);
                renderbuffer.memory = VK_NULL_HANDLE;
            }
        }

    }

    rendering_renderbuffer::~rendering_renderbuffer()
    {
        if (ownership.owns()) {
            _destroy_renderbuffer_image(*this);
        }
    }

    rendering_renderbuffer::rendering_renderbuffer(const uint32x2 size, const uint32 internal_format, const uint32 samples)
        : internal_format(static_cast<VkFormat>(internal_format))
        , sampling_count(samples)
        , size(size)
    {
        _create_renderbuffer_image(*this);
        ownership.emplace();
    }

    void rendering_renderbuffer::resize(const uint32x2 new_size)
    {
        if (size == new_size) {
            return;
        }
        _destroy_renderbuffer_image(*this);
        size = new_size;
        _create_renderbuffer_image(*this);
    }

}
}

#endif
