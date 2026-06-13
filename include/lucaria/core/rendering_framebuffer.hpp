#pragma once

#include <lucaria/core/rendering_renderbuffer.hpp>

namespace lucaria {
namespace detail {

    struct rendering_framebuffer {
        rendering_framebuffer(const rendering_framebuffer& other) = delete;
        rendering_framebuffer& operator=(const rendering_framebuffer& other) = delete;
        rendering_framebuffer(rendering_framebuffer&& other) = default;
        rendering_framebuffer& operator=(rendering_framebuffer&& other) = default;
        ~rendering_framebuffer();

        rendering_framebuffer();
        static void use_default();
        void use();
        void bind_color(const rendering_texture& color);
        void bind_color(rendering_renderbuffer& color);
        void bind_depth(rendering_texture& depth);
        void bind_depth(rendering_renderbuffer& depth);

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
        std::optional<GLuint> texture_color_id = std::nullopt;
        std::optional<GLuint> texture_depth_id = std::nullopt;
        std::optional<GLuint> renderbuffer_color_id = std::nullopt;
        std::optional<GLuint> renderbuffer_depth_id = std::nullopt;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        void* color = nullptr;
        void* depth = nullptr;
        uint32x2 size {};
        int psm = GU_PSM_8888;
        int fbw = 512;
        bool has_color = false;
        bool has_depth = false;
#endif
    };

}
}
