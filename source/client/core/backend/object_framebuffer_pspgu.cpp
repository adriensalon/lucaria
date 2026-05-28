#include <lucaria/core/framebuffer.hpp>

namespace lucaria {
namespace detail {

    object_framebuffer::object_framebuffer(object_framebuffer&& other)
    {
    }

    object_framebuffer& object_framebuffer::operator=(object_framebuffer&& other)
    {
        return *this;
    }

    object_framebuffer::~object_framebuffer()
    {
    }

    object_framebuffer::object_framebuffer()
    {
    }

    void object_framebuffer::use_default()
    {
    }

    void object_framebuffer::use()
    {
    }

    void object_framebuffer::bind_color(const object_texture& color)
    {
    }

    void object_framebuffer::bind_color(object_renderbuffer& color)
    {
    }

    void object_framebuffer::bind_depth(object_texture& depth)
    {
    }

    void object_framebuffer::bind_depth(object_renderbuffer& depth)
    {
    }

}
}