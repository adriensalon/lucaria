#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_motion_track.hpp>

namespace lucaria {

struct handle_motion_track : handle_asset<detail::object_motion_track> {
    using handle_asset<detail::object_motion_track>::handle_asset;

    friend struct context_object;
    friend struct component_animator;
    friend struct detail::system_motion;
    friend class cereal::access;
};

}
