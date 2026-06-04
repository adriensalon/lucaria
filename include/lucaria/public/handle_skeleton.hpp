#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_skeleton.hpp>

namespace lucaria {

struct handle_skeleton : handle_asset<detail::object_skeleton> {
    using handle_asset<detail::object_skeleton>::handle_asset;

    friend struct context_object;
    friend struct component_animator;
    friend struct detail::system_motion;
    friend class cereal::access;
};

}
