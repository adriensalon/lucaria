#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    [[nodiscard]] std::filesystem::path resolve_geometry_lod0(const std::filesystem::path& path);

    enum struct object_geometry_origin {
        path,
        data
    };

    struct asset_geometry {
        asset_geometry() = default;
        asset_geometry(const asset_geometry& other) = delete;
        asset_geometry& operator=(const asset_geometry& other) = delete;
        asset_geometry(asset_geometry&& other) = default;
        asset_geometry& operator=(asset_geometry&& other) = default;

        asset_geometry(const std::vector<char>& bytes);
        asset_geometry(data_geometry&& data);

        object_geometry_origin origin;
        std::filesystem::path origin_path;
        data_geometry data;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);
    };
}

struct handle_geometry : handle_asset<detail::asset_geometry> {
    using handle_asset<detail::asset_geometry>::handle_asset;
};

}
