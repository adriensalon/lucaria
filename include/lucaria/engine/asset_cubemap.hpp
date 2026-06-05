#pragma once

#include <lucaria/core/utils_owning.hpp>
#include <lucaria/engine/asset_image.hpp>
#include <lucaria/forward/handle_asset.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

#include <lucaria/core/serialize_context.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct manager_assets;

    enum struct object_cubemap_origin {
        path,
        data
    };

    struct object_cubemap {
        object_cubemap() = default;
        object_cubemap(const object_cubemap& other) = delete;
        object_cubemap& operator=(const object_cubemap& other) = delete;
        object_cubemap(object_cubemap&& other) = default;
        object_cubemap& operator=(object_cubemap&& other) = default;
        ~object_cubemap();

        object_cubemap(const std::array<object_image, 6>& images);

        object_cubemap_origin origin;
        data_image_profile profile;
        std::array<std::filesystem::path, 6> origin_paths;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            context.field("profile", profile);
            if (origin == object_cubemap_origin::path) {
                context.field("origin_path", origin_paths);
            }
        }

        void load(storage_load_context& context)
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

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        std::array<object_texture_pspgu, 6> faces = {};
#endif
    };
}

struct handle_cubemap : handle_asset<detail::object_cubemap> {
    using handle_asset<detail::object_cubemap>::handle_asset;
};

}
