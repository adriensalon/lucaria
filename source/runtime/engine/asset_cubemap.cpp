#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/engine/asset_cubemap.hpp>

namespace lucaria {
namespace detail {

    void object_cubemap::save(storage_save_context& context) const
    {
        context.field("origin", origin);
        context.field("profile", profile);
        if (origin == object_cubemap_origin::path) {
            context.field("origin_path", origin_paths);
        }
    }

    void object_cubemap::load(storage_load_context& context)
    {
        context.field("origin", origin);
        context.field("profile", profile);
        if (origin == object_cubemap_origin::path) {
            context.field("origin_path", origin_paths);
            const std::array<std::filesystem::path, 6> _paths = origin_paths;
            const data_image_profile _profile = profile;
            const std::array<std::filesystem::path, 6> _resolved_paths = resolve_profile(context.objects, _paths, _profile);
            context.fetch(_resolved_paths, [this, _paths](const std::vector<std::vector<char>>& bytes) {
                std::array<object_image, 6> _images = {
                    object_image(bytes[0]),
                    object_image(bytes[1]),
                    object_image(bytes[2]),
                    object_image(bytes[3]),
                    object_image(bytes[4]),
                    object_image(bytes[5])
                };
                *this = object_cubemap(_images);
                origin_paths = _paths;
            });
        }
    }

}
}
