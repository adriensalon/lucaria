#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_sound_track.hpp>

namespace lucaria {

struct handle_sound_track : handle_asset<detail::object_sound_track> {
    using handle_asset<detail::object_sound_track>::handle_asset;

    friend struct context_object;
    friend struct component_speaker_spatial;
    friend class cereal::access;
};

}
