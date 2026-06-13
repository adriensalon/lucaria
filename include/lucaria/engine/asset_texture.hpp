#pragma once

#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/engine/asset_image.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    enum struct object_texture_origin {
        path,
        data,
        size
    };

    struct asset_texture {
        asset_texture() = default;
        asset_texture(const asset_texture& other) = delete;
        asset_texture& operator=(const asset_texture& other) = delete;
        asset_texture(asset_texture&& other) = default;
        asset_texture& operator=(asset_texture&& other) = default;

        asset_texture(const asset_image& image);
        asset_texture(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const asset_image& image);
        [[nodiscard]] ImTextureID imgui_texture() const;

        object_texture_origin origin;
        std::filesystem::path origin_path;
		rendering_texture texture;

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };
}

struct handle_texture : handle_asset<detail::asset_texture> {
    using handle_asset<detail::asset_texture>::handle_asset;

    /// @brief Resizes the texture to the specified size
    /// @param size the new size of the texture
    void resize(const uint32x2 new_size);

    /// @brief Gets the size of the texture
    /// @return the size of the texture
    uint32x2 size() const;

    ImTextureID imgui_texture() const;
};

}
