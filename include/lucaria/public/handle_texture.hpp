#pragma once

#include <lucaria/public/handle_asset.hpp>
#include <lucaria/core/object_texture.hpp>

namespace lucaria {

/// @brief Represents a texture on the device. Can be created from an image file or from an empty size.
struct handle_texture : handle_asset<detail::object_texture> {
    using handle_asset<detail::object_texture>::handle_asset;

    /// @brief Resizes the texture to the specified size
    /// @param size the new size of the texture
    void resize(const uint32x2 new_size);

    /// @brief Gets the size of the texture
    /// @return the size of the texture
    uint32x2 size() const;

    ImTextureID imgui_texture() const;

    friend struct context_object;
    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
