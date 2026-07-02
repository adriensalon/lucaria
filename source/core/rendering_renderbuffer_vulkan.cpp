#if defined(LUCARIA_BACKEND_VULKAN)

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
            const VkImageAspectFlags _aspect = _is_depth_format(renderbuffer.internal_format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            const VkImageUsageFlags _usage = _is_depth_format(renderbuffer.internal_format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            rendering_vulkan_create_image(renderbuffer.size, renderbuffer.internal_format, _usage, _aspect, renderbuffer.image, renderbuffer.memory, renderbuffer.image_view, renderbuffer.layout);
        }

        void _destroy_renderbuffer_image(rendering_renderbuffer& renderbuffer)
        {
            rendering_vulkan_destroy_image(renderbuffer.image, renderbuffer.memory, renderbuffer.image_view);
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
