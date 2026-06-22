#pragma once

#include <lucaria/core/rendering_cubemap.hpp>
#include <lucaria/engine/asset_texture.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    enum struct asset_cubemap_origin {
        path,
        data
    };

    struct asset_cubemap {
        asset_cubemap() = default;
        asset_cubemap(const asset_cubemap& other) = delete;
        asset_cubemap& operator=(const asset_cubemap& other) = delete;
        asset_cubemap(asset_cubemap&& other) = default;
        asset_cubemap& operator=(asset_cubemap&& other) = default;

        asset_cubemap(const std::array<asset_image, 6>& images);

        asset_cubemap_origin origin;
        std::array<std::filesystem::path, 6> origin_paths;
		rendering_cubemap cubemap;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);
    };
}

struct handle_cubemap : handle_asset<detail::asset_cubemap> {
    using handle_asset<detail::asset_cubemap>::handle_asset;
};

}
