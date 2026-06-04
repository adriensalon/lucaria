#pragma once

#include <lucaria/core/object_renderbuffer.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

#include <lucaria/core/context_serialize.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_framebuffer {
        object_framebuffer(const object_framebuffer& other) = delete;
        object_framebuffer& operator=(const object_framebuffer& other) = delete;
        object_framebuffer(object_framebuffer&& other) = default;
        object_framebuffer& operator=(object_framebuffer&& other) = default;
        ~object_framebuffer();

        object_framebuffer();
        static void use_default();
        void use();
        void bind_color(const object_texture& color);
        void bind_color(object_renderbuffer& color);
        void bind_depth(object_texture& depth);
        void bind_depth(object_renderbuffer& depth);

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
