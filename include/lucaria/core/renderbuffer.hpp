#pragma once

#include <lucaria/core/texture.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/renderbuffer_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/renderbuffer_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

    struct renderbuffer_implementation {
        renderbuffer_implementation() = delete;
        renderbuffer_implementation(const renderbuffer_implementation& other) = delete;
        renderbuffer_implementation& operator=(const renderbuffer_implementation& other) = delete;
        renderbuffer_implementation(renderbuffer_implementation&& other);
        renderbuffer_implementation& operator=(renderbuffer_implementation&& other);
        ~renderbuffer_implementation();

        renderbuffer_implementation(const uint32x2 size, const glm::uint internal_format, const glm::uint samples = 1);
        void resize(const uint32x2 new_size);

#if defined(LUCARIA_BACKEND_OPENGL)
        renderbuffer_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        renderbuffer_implementation_pspgu implementation_pspgu;
#endif

        uint32 sampling_count;
        uint32x2 size;
    };

}
}
