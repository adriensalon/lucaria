#pragma once

#include <lucaria/core/rendering_cubemap.hpp>
#include <lucaria/engine/asset_texture.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

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

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };
}

struct handle_cubemap : handle_asset<detail::asset_cubemap> {
    using handle_asset<detail::asset_cubemap>::handle_asset;
};

}
