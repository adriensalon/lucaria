#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_geometry.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_geometry_bytes(data_geometry& data, const std::vector<char>& bytes)
        {
            bytes_stream _stream(bytes);
#if defined(LUCARIA_JSON_ASSETS)
            cereal::JSONInputArchive _archive(_stream);
#else
            cereal::PortableBinaryInputArchive _archive(_stream);
#endif
            _archive(data);
        }

        static container_async<object_geometry> _fetch_geometry_async(manager_object& objects, const std::filesystem::path& data_path)
        {
            std::shared_ptr<std::promise<object_geometry>> _promise = std::make_shared<std::promise<object_geometry>>();
            objects.fetch_bytes(data_path, [_promise](const std::vector<char>& _bytes) {
				object_geometry _geometry(_bytes);
				_promise->set_value(std::move(_geometry)); }, true);

            return container_async<object_geometry>(_promise->get_future());
        }
    }

    object_geometry::object_geometry(const std::vector<char>& bytes)
        : origin(object_geometry_origin::path)
    {
        _load_geometry_bytes(data, bytes);
    }

    object_geometry::object_geometry(data_geometry&& data)
        : origin(object_geometry_origin::data)
        , data(std::move(data))
    {
    }

	container_cache<object_geometry>& fetch(
		manager_object& objects, 
        container_cache_vector<object_geometry>& cached_vector,
		const std::filesystem::path& path)
	{
		return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_geometry_async(objects, path);
        });
	}

    recipe_object_geometry make_recipe(const container_cache<object_geometry>& cached)
    {
        const object_geometry& _geometry = cached.fetched.value();

        if (_geometry.origin == object_geometry_origin::path) {
            return recipe_object_geometry_path { cached.origin_path.value() };

        } else if (_geometry.origin == object_geometry_origin::data) {
            return recipe_object_geometry_data { _geometry.data };
			
        } else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

}
}
