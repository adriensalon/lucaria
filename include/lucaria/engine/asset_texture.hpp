#pragma once

#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/engine/asset_image.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

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
        [[nodiscard]] ImVec2 imgui_uv0() const;
        [[nodiscard]] ImVec2 imgui_uv1() const;

        object_texture_origin origin;
        std::filesystem::path origin_path;
		rendering_texture texture;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);

		inline static rendering_textures_registry* textures_registry = nullptr;
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
    ImVec2 imgui_uv0() const;
    ImVec2 imgui_uv1() const;
};

}
