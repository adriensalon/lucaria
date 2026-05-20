#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/shader.hpp>
#include <lucaria/core/texture.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/program_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/program_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

    struct program_implementation {
        LUCARIA_DELETE_DEFAULT(program_implementation)
        program_implementation(const program_implementation& other) = delete;
        program_implementation& operator=(const program_implementation& other) = delete;
        program_implementation(program_implementation&& other);
        program_implementation& operator=(program_implementation&& other);
        ~program_implementation();

        program_implementation(const shader& vertex, const shader& fragment);
        void use() const;
        void bind_attribute(const std::string& name, const mesh_implementation& mesh, const mesh_attribute attribute);
        void bind_uniform(const std::string& name, const cubemap_implementation& cubemap, const glm::uint slot = 0) const;
        void bind_uniform(const std::string& name, const texture_implementation& texture, const glm::uint slot = 0) const;
        template <typename T>
        void bind_uniform(const std::string& name, const T& value);
        void draw(const bool use_depth = true) const;

        void draw(const mesh_implementation& mesh); // + attributes select ?
        void bind_texture(const texture_implementation& mesh); // + attributes select ?
        void bind_cubemap(const cubemap_implementation& mesh); // + attributes select ?

#if defined(LUCARIA_DEBUG)
        void bind_guizmo(const std::string& name, const _detail::guizmo_mesh& from);
        void draw_guizmo() const;
#endif

        static void viewport(const uint32x2 size);
        static void clear(const bool clear_depth = false);

#if defined(LUCARIA_BACKEND_OPENGL)
        program_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        program_implementation_pspgu implementation_pspgu;
#endif
    };

}
}
