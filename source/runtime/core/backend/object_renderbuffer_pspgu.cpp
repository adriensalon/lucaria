#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {
namespace detail {

    rendering_renderbuffer::rendering_renderbuffer(rendering_renderbuffer&& other)
    {
    }

    rendering_renderbuffer& rendering_renderbuffer::operator=(rendering_renderbuffer&& other)
    {
        return *this;
    }

    rendering_renderbuffer::~rendering_renderbuffer()
    {
    }

    rendering_renderbuffer::rendering_renderbuffer(const uint32x2 size, const glm::uint internal_format, const glm::uint samples)
        : size(size)
    {
    }

    void rendering_renderbuffer::resize(const uint32x2 new_size)
    {
    }

}
}