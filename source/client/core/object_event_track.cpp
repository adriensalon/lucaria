#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _load_event_track_bytes(data_event_track& data, const std::vector<char>& bytes)
        {
            bytes_stream _stream(bytes);
#if defined(LUCARIA_JSON_ASSETS)
            cereal::JSONInputArchive _archive(_stream);
#else
            cereal::PortableBinaryInputArchive _archive(_stream);
#endif
            _archive(data);
#if defined(LUCARIA_DEBUG)
            std::cout << "Loaded event track" << std::endl;
#endif
        }

        static container_async<object_event_track> _fetch_event_track_async(manager_object& objects, const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_event_track>> _promise = std::make_shared<std::promise<object_event_track>>();
            objects.fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
				object_event_track _event_track(_bytes);
				_promise->set_value(std::move(_event_track)); }, true);

            // create event track on worker thread is ok
            return container_async<object_event_track>(_promise->get_future());
        }

    }

    object_event_track::object_event_track(const std::vector<char>& bytes)
        : origin(object_event_track_origin::path)
    {
        _load_event_track_bytes(data, bytes);
    }

    object_event_track::object_event_track(data_event_track&& data)
        : origin(object_event_track_origin::data)
        , data(std::move(data))
    {
    }

	container_cache<object_event_track>& fetch(
		manager_object& objects, 
        container_cache_vector<object_event_track>& cached_vector,
        const std::filesystem::path& path)
	{
		return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_event_track_async(objects, path);
        });
	}

    recipe_object_event_track make_recipe(const container_cache<object_event_track>& cached)
    {
        const object_event_track& _event_track = cached.fetched.value();

        if (_event_track.origin == object_event_track_origin::path) {
            return recipe_object_event_track_path { cached.origin_path.value() };
        }

        else if (_event_track.origin == object_event_track_origin::data) {
            return recipe_object_event_track_data { _event_track.data };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

}
}
