#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_mesh.hpp>

namespace lucaria {
namespace detail {

    asset_mesh::asset_mesh(const asset_geometry& geometry)
        : origin(geometry.origin == object_geometry_origin::path ? asset_mesh_origin::path : asset_mesh_origin::data)
        , mesh(*meshes_registry, geometry.data)
    {
        LUCARIA_DEBUG_ASSERT(meshes_registry, "asset_mesh::meshes_registry is null")
    }

    void asset_mesh::save(context_save_storage& context) const
    {
        context.field("origin", origin);
        if (origin == asset_mesh_origin::path) {
            context.field("origin_path", origin_path);
        }
    }

    void asset_mesh::load(context_load_storage& context)
    {
        context.field("origin", origin);
        if (origin == asset_mesh_origin::path) {
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                asset_geometry _geometry(bytes);
                *this = asset_mesh(_geometry);
                origin_path = _path;
            });
        }
    }

}
}
