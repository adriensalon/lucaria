#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/asset_cubemap.hpp>

namespace lucaria {
namespace detail {

    asset_cubemap::asset_cubemap(const std::array<asset_image, 6>& images)
        : origin(images[0].origin == object_image_origin::path ? asset_cubemap_origin::path : asset_cubemap_origin::data)
        , cubemap(images)
    {
    }

    void asset_cubemap::save(context_save_storage& context) const
    {
        context.field("origin", origin);
        context.field("profile", cubemap.profile);
        if (origin == asset_cubemap_origin::path) {
            context.field("origin_path", origin_paths);
        }
    }

    void asset_cubemap::load(context_load_storage& context)
    {
        context.field("origin", origin);
        context.field("profile", cubemap.profile);
        if (origin == asset_cubemap_origin::path) {
            context.field("origin_path", origin_paths);
            const std::array<std::filesystem::path, 6> _paths = origin_paths;
            const std::array<std::filesystem::path, 6> _resolved_paths = resolve_profile(context.objects, _paths, cubemap.profile);
            context.fetch(_resolved_paths, [this, _paths](const std::vector<std::vector<char>>& _bytes) {
                std::array<asset_image, 6> _images = {
                    asset_image(_bytes[0]),
                    asset_image(_bytes[1]),
                    asset_image(_bytes[2]),
                    asset_image(_bytes[3]),
                    asset_image(_bytes[4]),
                    asset_image(_bytes[5])
                };
                *this = asset_cubemap(_images);
                origin_paths = _paths;
            });
        }
    }

}
}
