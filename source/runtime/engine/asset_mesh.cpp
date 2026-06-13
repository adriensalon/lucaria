#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_mesh.hpp>

namespace lucaria {
namespace detail {

	asset_mesh::asset_mesh(const asset_geometry& geometry)
	{

	}

    void asset_mesh::save(storage_save_context& context) const
    {
        context.field("origin", origin);
        if (origin == asset_mesh_origin::path) {
            context.field("origin_path", origin_path);
        }
    }

    void asset_mesh::load(storage_load_context& context)
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
