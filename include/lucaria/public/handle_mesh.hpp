#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_mesh.hpp>

namespace lucaria {

struct handle_mesh : handle_asset<detail::object_mesh> {
    using handle_asset<detail::object_mesh>::handle_asset;

    friend struct context_object;
    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
