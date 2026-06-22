#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/asset_event_track.hpp>

namespace lucaria {
namespace detail {

    object_event_track::object_event_track(const std::vector<char>& bytes)
        : origin(object_event_track_origin::path)
    {
        assets_bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
    }

    object_event_track::object_event_track(data_event_track&& data)
        : origin(object_event_track_origin::data)
        , data(std::move(data))
    {
    }

    void object_event_track::save(context_save_storage& context) const
    {
        context.field("origin", origin);
        if (origin == object_event_track_origin::path) {
            context.field("origin_path", origin_path);
        }
        if (origin == object_event_track_origin::data) {
            context.field("origin_data", data);
        }
    }

    void object_event_track::load(context_load_storage& context)
    {
        context.field("origin", origin);
        if (origin == object_event_track_origin::path) {
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                *this = object_event_track(bytes);
                origin_path = _path;
            });
        }
        if (origin == object_event_track_origin::data) {
            context.field("origin_data", data);
        }
    }

}
}
