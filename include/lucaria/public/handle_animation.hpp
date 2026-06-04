#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_animation.hpp>

namespace lucaria {

struct handle_animation : handle_asset<detail::object_animation> {
    using handle_asset<detail::object_animation>::handle_asset;

    friend struct component_animator;
    friend struct context_object;
    friend struct detail::system_motion;
    friend class cereal::access;
};

}
