#include <lucaria/core/database.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/mesh.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static async_container<mesh_implementation> _fetch_mesh_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<geometry_implementation>> _geometry_promise = std::make_shared<std::promise<geometry_implementation>>();
            fetch_bytes(path, [_geometry_promise](const std::vector<char>& _data_bytes) {
        geometry_implementation _geometry(_data_bytes);
        _geometry_promise->set_value(std::move(_geometry)); }, true);

            // create mesh on main thread
            return async_container<mesh_implementation>(_geometry_promise->get_future(), [](const geometry_implementation& _from) {
                return mesh_implementation(_from);
            });
        }
    }

    mesh_recipe make_recipe(const implementation_container<mesh_implementation>& container)
    {
        const mesh_implementation& _mesh = container.fetched.value();

        if (_mesh.origin == mesh_origin::path) {
            return mesh_path_recipe { container.origin_path.value() };
        }

        // else if (_mesh.origin == mesh_origin::data) {
		// 	return {};
        //     // return mesh_data_recipe { geometry_implementation(_mesh).data };
        // }

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }
}

mesh_object mesh_object::fetch(const std::filesystem::path& path)
{
    detail::implementation_container<detail::mesh_implementation>* _resource = detail::engine_resources().meshes.get_or_create_by_path(path, [&] {
        return detail::_fetch_mesh_async(path);
    });

    return mesh_object { _resource };
}

bool mesh_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

mesh_object::operator bool() const
{
    return has_value();
}

mesh_object::mesh_object(detail::implementation_container<detail::mesh_implementation>* resource)
    : _resource(resource)
{
}

}
