#pragma once

#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

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

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);

		inline static rendering_meshes_registry* meshes_registry = nullptr;
    };
}

struct handle_mesh : handle_asset<detail::asset_mesh> {
    using handle_asset<detail::asset_mesh>::handle_asset;
};

}
