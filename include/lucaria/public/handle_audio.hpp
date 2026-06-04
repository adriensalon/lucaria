#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_audio.hpp>

namespace lucaria {

struct handle_audio : handle_asset<detail::object_audio> {
    using handle_asset<detail::object_audio>::handle_asset;

    friend struct context_object;
    friend class cereal::access;
};

}
