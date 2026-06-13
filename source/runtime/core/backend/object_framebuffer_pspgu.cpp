#include <lucaria/core/framebuffer.hpp>

namespace lucaria {
namespace detail {

    rendering_framebuffer::rendering_framebuffer(rendering_framebuffer&& other)
    {
    }

    rendering_framebuffer& rendering_framebuffer::operator=(rendering_framebuffer&& other)
    {
        return *this;
    }

    rendering_framebuffer::~rendering_framebuffer()
    {
    }

    rendering_framebuffer::rendering_framebuffer()
    {
    }

    void rendering_framebuffer::use_default()
    {
    }

    void rendering_framebuffer::use()
    {
    }

    void rendering_framebuffer::bind_color(const asset_texture& color)
    {
    }

    void rendering_framebuffer::bind_color(rendering_renderbuffer& color)
    {
    }

    void rendering_framebuffer::bind_depth(asset_texture& depth)
    {
    }

    void rendering_framebuffer::bind_depth(rendering_renderbuffer& depth)
    {
    }

}
}