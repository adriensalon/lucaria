#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_geometry.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_geometry::object_geometry(const std::vector<char>& bytes)
        : origin(object_geometry_origin::path)
    {
        bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    object_geometry::object_geometry(data_geometry&& data)
        : origin(object_geometry_origin::data)
        , data(std::move(data))
    {
    }

    assets_cell<object_geometry>& fetch(
        manager_assets& objects,
        assets_buffer<object_geometry>& cached_vector,
        const std::filesystem::path& path)
    {
        const std::string _cache_id = path.string();
        return *cached_vector.get_or_create_by_id(_cache_id, [&objects, path] {
            std::shared_ptr<std::promise<object_geometry>> _promise = std::make_shared<std::promise<object_geometry>>();
            objects.fetch_bytes(path, [_promise, path](const std::vector<char>& _bytes) {
				object_geometry _geometry(_bytes);
				_geometry.origin_path = path;
				_promise->set_value(std::move(_geometry)); }, true);

            return container_async<object_geometry>(_promise->get_future());
        });
    }

    recipe_object_geometry make_recipe(const assets_cell<object_geometry>& cached)
    {
        const object_geometry& _geometry = cached.fetched.value();

        if (_geometry.origin == object_geometry_origin::path) {
            return recipe_object_geometry_path { cached.fetched.value().origin_path.value() };

        } else if (_geometry.origin == object_geometry_origin::data) {
            return recipe_object_geometry_data { _geometry.data };

        } else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    assets_cell<object_geometry>* apply_recipe(manager_assets& objects, assets_buffer<object_geometry>& cached_vector, recipe_object_geometry& recipe)
    {
        return std::visit([&](auto& value) -> assets_cell<object_geometry>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_geometry_path>) {
                return &fetch(objects, cached_vector, value.path);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_geometry_data>) {
                return cached_vector.create_cell(
                    container_async<object_geometry>(
                        object_geometry(std::move(value.data))));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }

}
}
