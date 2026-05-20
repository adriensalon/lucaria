#pragma once

#include <lucaria/core/math.hpp>
#include <lucaria/core/owning.hpp>

#include <lucaria/core/backend/opengl/backend_opengl.hpp>

namespace lucaria {
namespace detail {

    struct cubemap_implementation_opengl {
		owning_flag ownership = {};
        GLuint id = 0;
    };

}
}