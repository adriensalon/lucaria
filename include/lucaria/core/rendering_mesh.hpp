#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/rendering_backend.hpp>
#include <lucaria/core/utils_owning.hpp>

namespace lucaria {
namespace detail {

    enum struct rendering_mesh_attribute {
        position,
        color,
        normal,
        tangent,
        bitangent,
        texcoord,
        bones,
        weights
    };

    struct rendering_mesh {
        rendering_mesh() = default;
        rendering_mesh(const rendering_mesh& other) = delete;
        rendering_mesh& operator=(const rendering_mesh& other) = delete;
        rendering_mesh(rendering_mesh&& other) = default;
        rendering_mesh& operator=(rendering_mesh&& other) = default;
		~rendering_mesh();

        rendering_mesh(const data_geometry& geometry);

        std::vector<float32x4x4> invposes;
        uint32 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint array_id = 0;
        GLuint elements_id = 0;
        std::unordered_map<rendering_mesh_attribute, GLuint> attribute_ids = {};
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        void* vertices = nullptr; // aligned CPU memory
        uint32 vertex_count = 0;
        int vertex_type = 0; // GU_VERTEX_32BITF | GU_TEXTURE_32BITF...
        int primitive = GU_TRIANGLES;
#endif

    private:
        flag_owning _ownership = {};
    };

    struct rendering_mesh_line {
        rendering_mesh_line() = delete;
        rendering_mesh_line(const rendering_mesh_line& other) = delete;
        rendering_mesh_line& operator=(const rendering_mesh_line& other) = delete;
        rendering_mesh_line(rendering_mesh_line&& other) = default;
        rendering_mesh_line& operator=(rendering_mesh_line&& other) = default;
        ~rendering_mesh_line();

        rendering_mesh_line(const data_geometry& from);
        rendering_mesh_line(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
        void update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);

        uint32 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint array_handle = 0;
        GLuint elements_handle = 0;
        GLuint positions_handle = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#endif

    private:
        flag_owning _ownership = {};
    };
}

}
