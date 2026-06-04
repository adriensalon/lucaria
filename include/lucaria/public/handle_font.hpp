#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_font.hpp>

namespace lucaria {

struct handle_font : handle_asset<detail::object_font> {
    using handle_asset<detail::object_font>::handle_asset;

    ImFont* imgui_font() const;

    friend struct context_object;
    friend class cereal::access;
};

}
