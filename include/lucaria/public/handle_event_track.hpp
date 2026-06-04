#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_event_track.hpp>

namespace lucaria {

struct handle_event_track : handle_asset<detail::object_event_track> {
    using handle_asset<detail::object_event_track>::handle_asset;

    friend struct context_object;
    friend struct component_animator;
    friend struct detail::system_motion;
    friend class cereal::access;
};

}
