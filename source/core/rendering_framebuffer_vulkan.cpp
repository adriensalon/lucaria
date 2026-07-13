#if defined(LUCARIA_BACKEND_VULKAN)

#include <lucaria/core/rendering_framebuffer.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {

    rendering_framebuffer::~rendering_framebuffer() = default;

    rendering_framebuffer::rendering_framebuffer()
    {
        ownership.emplace();
    }

    void rendering_framebuffer::use_default()
    {
        rendering_vulkan_use_default_target();
    }

    void rendering_framebuffer::use()
    {
        if (texture_color != nullptr) {
            VkImageLayout* _color_layout = &const_cast<rendering_texture*>(texture_color)->layout;
            if (texture_depth != nullptr) {
                rendering_texture* _depth = const_cast<rendering_texture*>(texture_depth);
                rendering_vulkan_use_target(texture_color->image, texture_color->image_view, texture_color->format, _color_layout, texture_color->size, _depth->image, _depth->image_view, _depth->format, &_depth->layout);
            } else if (renderbuffer_depth != nullptr) {
                rendering_renderbuffer* _depth = const_cast<rendering_renderbuffer*>(renderbuffer_depth);
                rendering_vulkan_use_target(texture_color->image, texture_color->image_view, texture_color->format, _color_layout, texture_color->size, _depth->image, _depth->image_view, _depth->internal_format, &_depth->layout);
            } else {
                rendering_vulkan_use_target(texture_color->image, texture_color->image_view, texture_color->format, _color_layout, texture_color->size);
            }
        } else if (renderbuffer_color != nullptr) {
            rendering_renderbuffer* _color = const_cast<rendering_renderbuffer*>(renderbuffer_color);
            if (renderbuffer_depth != nullptr) {
                rendering_renderbuffer* _depth = const_cast<rendering_renderbuffer*>(renderbuffer_depth);
                rendering_vulkan_use_target(_color->image, _color->image_view, _color->internal_format, &_color->layout, _color->size, _depth->image, _depth->image_view, _depth->internal_format, &_depth->layout);
            } else {
                rendering_vulkan_use_target(_color->image, _color->image_view, _color->internal_format, &_color->layout, _color->size);
            }
        }
    }

    void rendering_framebuffer::bind_color(rendering_texture& color)
    {
        texture_color = &color;
        renderbuffer_color = nullptr;
    }

    void rendering_framebuffer::bind_color(rendering_renderbuffer& color)
    {
        renderbuffer_color = &color;
        texture_color = nullptr;
    }

    void rendering_framebuffer::bind_depth(rendering_texture& depth)
    {
        texture_depth = &depth;
        renderbuffer_depth = nullptr;
    }

    void rendering_framebuffer::bind_depth(rendering_renderbuffer& depth)
    {
        renderbuffer_depth = &depth;
        texture_depth = nullptr;
    }

}
}

#endif
