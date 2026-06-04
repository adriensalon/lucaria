#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_cubemap.hpp>

namespace lucaria {

struct handle_cubemap : handle_asset<detail::object_cubemap> {
    using handle_asset<detail::object_cubemap>::handle_asset;

    friend struct context_object;
    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
