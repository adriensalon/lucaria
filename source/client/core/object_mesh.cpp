#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static container_async<object_mesh> _fetch_mesh_async(manager_assets& objects, const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_geometry>> _geometry_promise = std::make_shared<std::promise<object_geometry>>();
            objects.fetch_bytes(path, [_geometry_promise](const std::vector<char>& _data_bytes) {
				object_geometry _geometry(_data_bytes);
				_geometry_promise->set_value(std::move(_geometry)); }, true);

            // create mesh on main thread
            return container_async<object_mesh>(_geometry_promise->get_future(), [](const object_geometry& _from) {
                return object_mesh(_from);
            });
        }
    }

    container_cache<object_mesh>& fetch(
        manager_assets& objects,
        container_cache_vector<object_mesh>& cached_vector,
        const std::filesystem::path& path)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_mesh_async(objects, path);
        });
    }

    recipe_object_mesh make_recipe(const container_cache<object_mesh>& cached)
    {
        const object_mesh& _mesh = cached.fetched.value();

        if (_mesh.origin == object_mesh_origin::path) {
            return recipe_object_mesh_path { cached.origin_path.value() };
        }

        // else if (_mesh.origin == object_mesh_origin::data) {
        // 	return {};
        //     // return recipe_object_mesh_data { object_geometry(_mesh).data };
        // }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    container_cache<object_mesh>* apply_recipe(manager_assets& objects, container_cache_vector<object_mesh>& cached_vector, recipe_object_mesh& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_mesh>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_mesh_path>) {
                return &fetch(objects, cached_vector, value.path);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_mesh_data>) {
                return cached_vector.create_cell(
                    container_async<object_mesh>(
                        object_mesh(std::move(value.data))));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }
}
}
