#pragma once

#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    enum struct asset_mesh_origin {
        path,
        data
    };

    struct asset_mesh {
        asset_mesh() = default;
        asset_mesh(const asset_mesh& other) = delete;
        asset_mesh& operator=(const asset_mesh& other) = delete;
        asset_mesh(asset_mesh&& other) = default;
        asset_mesh& operator=(asset_mesh&& other) = default;

        asset_mesh(const asset_geometry& geometry);

        asset_mesh_origin origin;
        std::filesystem::path origin_path;
		rendering_mesh mesh;

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };
}

struct handle_mesh : handle_asset<detail::asset_mesh> {
    using handle_asset<detail::asset_mesh>::handle_asset;
};

}
