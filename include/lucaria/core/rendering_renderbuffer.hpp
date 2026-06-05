#pragma once

#include <lucaria/engine/asset_texture.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_renderbuffer {
        object_renderbuffer() = delete;
        object_renderbuffer(const object_renderbuffer& other) = delete;
        object_renderbuffer& operator=(const object_renderbuffer& other) = delete;
        object_renderbuffer(object_renderbuffer&& other) = default;
        object_renderbuffer& operator=(object_renderbuffer&& other) = default;
        ~object_renderbuffer();

        object_renderbuffer(const uint32x2 size, const uint32 internal_format, const uint32 samples = 1);
        void resize(const uint32x2 new_size);

#if defined(LUCARIA_BACKEND_OPENGL)
		flag_owning ownership = {};
        GLuint id = 0;
        GLuint internal_format = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        bool is_owning = false;
        void* pixels = nullptr;
		int psm = GU_PSM_8888;
		int fbw = 512;
#endif

        uint32 sampling_count;
        uint32x2 size;
    };

}
}
