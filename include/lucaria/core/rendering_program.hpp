#pragma once

#include <lucaria/core/rendering_backend.hpp>
#include <lucaria/core/rendering_shader.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_cubemap.hpp>
#include <lucaria/core/rendering_mesh.hpp>

namespace lucaria {
namespace detail {

    struct rendering_program {
        rendering_program() = delete;
        rendering_program(const rendering_program& other) = delete;
        rendering_program& operator=(const rendering_program& other) = delete;
        rendering_program(rendering_program&& other) = default;
        rendering_program& operator=(rendering_program&& other) = default;
        ~rendering_program();

        rendering_program(const object_shader& vertex, const object_shader& fragment);
        void use() const;
        void bind_attribute(const std::string& name, const rendering_mesh& mesh, const rendering_mesh_attribute attribute);
        void bind_uniform(const std::string& name, const rendering_cubemap& cubemap, const uint32 slot = 0) const;
        void bind_uniform(const std::string& name, const rendering_texture& texture, const uint32 slot = 0) const;
        template <typename T>
        void bind_uniform(const std::string& name, const T& value);
        void draw(const bool use_depth = true) const;

        static void viewport(const uint32x2 size);
        static void clear(const bool clear_depth = false);

#if defined(LUCARIA_DEBUG)
        void bind_guizmo(const std::string& name, const rendering_mesh_line& from);
        void draw_guizmo() const;
#endif

#if defined(LUCARIA_BACKEND_OPENGL)
        flag_owning ownership = {};
        GLuint id = 0;
        std::unordered_map<std::string, GLint> reflected_attributes = {};
        std::unordered_map<std::string, GLint> reflected_uniforms = {};
        GLuint bound_array_id = 0;
        GLuint bound_indices_count = 0;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        const object_mesh_pspgu* mesh = nullptr;
        const object_texture_pspgu* texture = nullptr;
        bool texture_enabled = false;
        bool lighting_enabled = false;
        bool depth_enabled = true;
#endif
    };

}
}
