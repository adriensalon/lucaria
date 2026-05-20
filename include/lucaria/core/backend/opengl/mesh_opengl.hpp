#pragma once

#include <unordered_map>

#include <lucaria/core/math.hpp>
#include <lucaria/core/owning.hpp>

#include <lucaria/core/backend/opengl/backend_opengl.hpp>

namespace lucaria {
namespace detail {

    enum struct mesh_attribute;

    struct mesh_implementation_opengl {
		owning_flag ownership = {};
        GLuint array_id = 0;
        GLuint elements_id = 0;
        std::unordered_map<mesh_attribute, GLuint> attribute_ids = {};
    };

}
}