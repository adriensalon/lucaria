#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_image.hpp>

namespace lucaria {

/// @brief Represents an image in CPU memory. Can be created from an image file or from an empty size.
struct handle_image : handle_asset<detail::object_image> {
    using handle_asset<detail::object_image>::handle_asset;

    /// @brief Resizes the image to the specified size
    /// @param size the new size of the image
    void resize(const uint32x2 size);

    /// @brief Gets the size of the image
    /// @return the size of the image
    uint32x2 get_size() const;

    friend struct context_object;
    friend class cereal::access;
};

}
