#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {
namespace detail {

    object_renderbuffer::object_renderbuffer(object_renderbuffer&& other)
    {
    }

    object_renderbuffer& object_renderbuffer::operator=(object_renderbuffer&& other)
    {
        return *this;
    }

    object_renderbuffer::~object_renderbuffer()
    {
    }

    object_renderbuffer::object_renderbuffer(const uint32x2 size, const glm::uint internal_format, const glm::uint samples)
        : size(size)
    {
    }

    void object_renderbuffer::resize(const uint32x2 new_size)
    {
    }

}
}