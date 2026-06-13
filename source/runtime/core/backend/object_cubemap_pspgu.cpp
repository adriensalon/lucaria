#if defined(LUCARIA_BACKEND_PSPGU)

#include <lucaria/core/cubemap.hpp>

namespace lucaria {
namespace detail {

    asset_cubemap::asset_cubemap(asset_cubemap&& other)
    {
    }

    asset_cubemap& asset_cubemap::operator=(asset_cubemap&& other)
    {
        return *this;
    }

    asset_cubemap::~asset_cubemap()
    {
    }

    asset_cubemap::asset_cubemap(const std::array<asset_image, 6>& images)
    {
    }

}
}

#endif