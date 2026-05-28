#if defined(LUCARIA_BACKEND_PSPGU)

#include <lucaria/core/cubemap.hpp>

namespace lucaria {
namespace detail {

    object_cubemap::object_cubemap(object_cubemap&& other)
    {
    }

    object_cubemap& object_cubemap::operator=(object_cubemap&& other)
    {
        return *this;
    }

    object_cubemap::~object_cubemap()
    {
    }

    object_cubemap::object_cubemap(const std::array<object_image, 6>& images)
    {
    }

}
}

#endif