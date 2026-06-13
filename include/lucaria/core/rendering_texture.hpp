#pragma once

#include <imgui.h>

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/rendering_backend.hpp>
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

        rendering_texture(const data_image& image);
        rendering_texture(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const data_image& image);
        [[nodiscard]] ImTextureID imgui_texture() const;

        data_image_profile profile;
        uint32x2 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
		// TODO
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        void* pixels = nullptr; // VRAM or memalign(16)
        int psm = GU_PSM_8888;
        int tbw = 0; // texture buffer width
        bool is_swizzled = false;
#endif

    private:
        flag_owning _ownership = {};
    };
}
}
