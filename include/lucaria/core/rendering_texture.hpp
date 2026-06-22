#pragma once

#include <imgui.h>

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/rendering_storage.hpp>
#include <lucaria/core/utils_owning.hpp>

namespace lucaria {
namespace detail {

    struct rendering_texture {
        rendering_texture() = default;
        rendering_texture(const rendering_texture& other) = delete;
        rendering_texture& operator=(const rendering_texture& other) = delete;
        rendering_texture(rendering_texture&& other) = default;
        rendering_texture& operator=(rendering_texture&& other) = default;
        ~rendering_texture();

        rendering_texture(rendering_textures_registry& registry, const data_image& image);
        rendering_texture(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const data_image& image);
        [[nodiscard]] ImTextureID imgui_texture() const;
        [[nodiscard]] ImVec2 imgui_uv0() const;
        [[nodiscard]] ImVec2 imgui_uv1() const;

        data_image_profile profile = data_image_profile::rgba8888;
        uint32x2 size = {};
        bool is_dedicated_storage = true;
        rendering_texture_allocation allocation = {};
        float32x4 uv_rect = { 0.f, 0.f, 1.f, 1.f };

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint texture_id = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#endif

    private:
        void _release() noexcept;

        rendering_textures_registry* _registry = nullptr;
        flag_owning _ownership = {};
    };
}
}
