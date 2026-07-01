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
        for (std::size_t _index = 0; _index < images.size(); ++_index) {
            faces[_index].emplace(uint32x2(images[_index].width, images[_index].height));
            faces[_index]->update(images[_index]);
        }
        _ownership.emplace();
    }

    rendering_cubemap::rendering_cubemap(const std::array<asset_image, 6>& images)
    {
        if (!images.empty()) {
            profile = images[0].data.profile;
            size = { images[0].data.width, images[0].data.height };
        }
        for (std::size_t _index = 0; _index < images.size(); ++_index) {
            faces[_index].emplace(uint32x2(images[_index].data.width, images[_index].data.height));
            faces[_index]->update(images[_index].data);
        }
        _ownership.emplace();
    }

}
}
