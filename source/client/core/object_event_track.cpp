#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    object_event_track::object_event_track(const std::vector<char>& bytes)
        : origin(object_event_track_origin::path)
    {
        bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    object_event_track::object_event_track(data_event_track&& data)
        : origin(object_event_track_origin::data)
        , data(std::move(data))
    {
    }

}
}
