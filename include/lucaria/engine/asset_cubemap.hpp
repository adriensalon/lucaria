#pragma once

#include <lucaria/core/utils_owning.hpp>
#include <lucaria/engine/asset_image.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif


namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

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

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);

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
