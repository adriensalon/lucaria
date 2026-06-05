#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

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

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };
}

struct handle_geometry : handle_asset<detail::asset_geometry> {
    using handle_asset<detail::asset_geometry>::handle_asset;
};

}
