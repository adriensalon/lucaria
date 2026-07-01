#pragma once

#include <array>
#include <optional>

#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/engine/asset_image.hpp>

namespace lucaria {
namespace detail {

    struct rendering_cubemap {
        rendering_cubemap() = default;
        rendering_cubemap(const rendering_cubemap& other) = delete;
        rendering_cubemap& operator=(const rendering_cubemap& other) = delete;
        rendering_cubemap(rendering_cubemap&& other) = default;
        rendering_cubemap& operator=(rendering_cubemap&& other) = default;
        ~rendering_cubemap();

        rendering_cubemap(const std::array<data_image, 6>& images);
        rendering_cubemap(const std::array<asset_image, 6>& images);

        data_image_profile profile;
        uint32x2 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint id = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
        // TODO
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        std::array<std::optional<rendering_texture>, 6> faces = {};
#endif

    private:
        flag_owning _ownership = {};
    };

}
}
