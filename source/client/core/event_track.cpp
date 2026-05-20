#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/event_track.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/stream.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace detail {	
	
    namespace {

        static void _load_event_track_bytes(event_track_data& data, const std::vector<char>& bytes)
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

        static async_container<event_track_implementation> _fetch_event_track_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<event_track_implementation>> _promise = std::make_shared<std::promise<event_track_implementation>>();
            fetch_bytes(path, [_promise](const std::vector<char>& _bytes) {
        event_track_implementation _event_track(_bytes);
        _promise->set_value(std::move(_event_track)); }, true);

            // create event track on worker thread is ok
            return async_container<event_track_implementation>(_promise->get_future());
        }

    }

    event_track_implementation::event_track_implementation(const std::vector<char>& bytes)
    {
        _load_event_track_bytes(data, bytes);
    }

    event_track_implementation::event_track_implementation(event_track_data&& data)
        : data(std::move(data))
    {
    }

}

event_track_object event_track_object::fetch(const std::filesystem::path& path)
{
    detail::resource_container<detail::event_track_implementation>* _resource = detail::engine_resources().event_tracks.get_or_create_by_path(path, [&] {
        return detail::_fetch_event_track_async(path);
    });

    return event_track_object { _resource };
}

bool event_track_object::has_value() const
{
    return _resource && _resource->is_ready();
}

event_track_object::operator bool() const
{
    return has_value();
}

event_track_object::event_track_object(detail::resource_container<detail::event_track_implementation>* resource)
    : _resource(resource)
{
}

}
