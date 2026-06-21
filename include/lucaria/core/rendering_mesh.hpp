#pragma once

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/rendering_storage.hpp>
#include <lucaria/core/utils_owning.hpp>

namespace lucaria {
namespace detail {

    struct rendering_mesh {
        rendering_mesh() = default;
        rendering_mesh(const rendering_mesh& other) = delete;
        rendering_mesh& operator=(const rendering_mesh& other) = delete;
        rendering_mesh(rendering_mesh&& other) = default;
        rendering_mesh& operator=(rendering_mesh&& other) = default;
        ~rendering_mesh();

        rendering_mesh(rendering_mesh_registry& registry, const data_geometry& geometry);

        std::vector<float32x4x4> invposes;
        data_geometry_profile profile = 0;
        uint32 size = 0;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint array_id = 0;
        GLuint elements_id = 0;
        GLuint vertices_id = 0;
        uint32 geometry_page = 0;
        uint32 vertex_offset = 0;
        uint32 vertex_size = 0;
        uint32 element_offset = 0;
        uint32 element_size = 0;
        uint32 vertex_stride = 0;
        std::unordered_map<data_vertex_attribute, uint32> attribute_offsets = {};
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
		rendering_mesh_registry* _registry = nullptr;
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
