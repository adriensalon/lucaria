#include <lucaria/core/rendering_cubemap.hpp>

namespace lucaria {
namespace detail {

    rendering_cubemap::~rendering_cubemap() = default;

    rendering_cubemap::rendering_cubemap(const std::array<data_image, 6>& images)
    {
        if (!images.empty()) {
            profile = images[0].profile;
            size = { images[0].width, images[0].height };
        }
        _ownership.emplace();
    }

    rendering_cubemap::rendering_cubemap(const std::array<asset_image, 6>& images)
    {
        if (!images.empty()) {
            profile = images[0].data.profile;
            size = { images[0].data.width, images[0].data.height };
        }
        _ownership.emplace();
    }

}
}
