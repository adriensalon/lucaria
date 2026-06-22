#pragma once

#include <lucaria/bin/data_event_track.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

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

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);

    };
}

struct handle_event_track : handle_asset<detail::object_event_track> {
    using handle_asset<detail::object_event_track>::handle_asset;
};

}
