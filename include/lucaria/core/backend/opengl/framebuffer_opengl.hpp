#pragma once

#include <optional>

#include <lucaria/core/math.hpp>
#include <lucaria/core/owning.hpp>

#include <lucaria/core/backend/opengl/backend_opengl.hpp>

namespace lucaria {
namespace detail {

    struct framebuffer_implementation_opengl {
		owning_flag ownership = {};
        GLuint id = 0;
        std::optional<GLuint> texture_color_id = std::nullopt;
        std::optional<GLuint> texture_depth_id = std::nullopt;
        std::optional<GLuint> renderbuffer_color_id = std::nullopt;
        std::optional<GLuint> renderbuffer_depth_id = std::nullopt;
    };

}
}