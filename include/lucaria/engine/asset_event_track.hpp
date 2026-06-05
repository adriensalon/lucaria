#pragma once

#include <lucaria/bin/data_event_track.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    enum struct object_event_track_origin {
        path,
		data
    };

    struct object_event_track {
        object_event_track() = default;
        object_event_track(const object_event_track& other) = delete;
        object_event_track& operator=(const object_event_track& other) = delete;
        object_event_track(object_event_track&& other) = default;
        object_event_track& operator=(object_event_track&& other) = default;

        object_event_track(const std::vector<char>& bytes);
        object_event_track(data_event_track&& data);

        object_event_track_origin origin;
		std::filesystem::path origin_path;		
        data_event_track data;

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);

    };
}

struct handle_event_track : handle_asset<detail::object_event_track> {
    using handle_asset<detail::object_event_track>::handle_asset;
};

}
