#pragma once

#include <lucaria/core/renderbuffer.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/framebuffer_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/framebuffer_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

    struct framebuffer_implementation {
        framebuffer_implementation(const framebuffer_implementation& other) = delete;
        framebuffer_implementation& operator=(const framebuffer_implementation& other) = delete;
        framebuffer_implementation(framebuffer_implementation&& other);
        framebuffer_implementation& operator=(framebuffer_implementation&& other);
        ~framebuffer_implementation();

        framebuffer_implementation();
        static void use_default();
        void use();
        void bind_color(const texture_implementation& color);
        void bind_color(renderbuffer_implementation& color);
        void bind_depth(texture_implementation& depth);
        void bind_depth(renderbuffer_implementation& depth);

#if defined(LUCARIA_BACKEND_OPENGL)
        framebuffer_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        framebuffer_implementation_pspgu implementation_pspgu;
#endif
    };

}
}
