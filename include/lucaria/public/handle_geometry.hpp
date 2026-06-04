#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_geometry.hpp>

namespace lucaria {

/// @brief Represents a geometry on the device. Can be created from a geometry file.
struct handle_geometry : handle_asset<detail::object_geometry> {
    using handle_asset<detail::object_geometry>::handle_asset;

    friend struct context_object;
    friend struct detail::system_rendering;
    friend struct handle_shape;
    friend struct component_interface_spatial;
    friend class cereal::access;
};

}
