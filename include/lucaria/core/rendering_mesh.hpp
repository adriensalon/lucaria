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

        rendering_mesh(rendering_meshes_registry& registry, const data_geometry& geometry);

        std::vector<float32x4x4> invposes;
        data_geometry_profile profile = 0;
        uint32 size = 0;
        rendering_mesh_allocation allocation = {};
        uint32 vertex_stride = 0;
        std::unordered_map<data_vertex_attribute, uint32> attribute_offsets = {};

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint array_id = 0;
        GLuint elements_id = 0;
        GLuint vertices_id = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
        VkBuffer vertices_buffer = VK_NULL_HANDLE;
        VkDeviceMemory vertices_memory = VK_NULL_HANDLE;
        VkBuffer elements_buffer = VK_NULL_HANDLE;
        VkDeviceMemory elements_memory = VK_NULL_HANDLE;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        std::vector<uint8> vertex_bytes = {};
        std::vector<uint16> elements_16 = {};
        int vertex_format = 0;
#endif

    private:
        rendering_meshes_registry* _registry = nullptr;
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
        rendering_mesh_line(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices);
        void update(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices);

        uint32 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint array_handle = 0;
        GLuint elements_handle = 0;
        GLuint positions_handle = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
        VkBuffer positions_buffer = VK_NULL_HANDLE;
        VkDeviceMemory positions_memory = VK_NULL_HANDLE;
        VkBuffer elements_buffer = VK_NULL_HANDLE;
        VkDeviceMemory elements_memory = VK_NULL_HANDLE;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        std::vector<float32x3> positions = {};
        std::vector<uint16> elements_16 = {};
#endif

    private:
        flag_owning _ownership = {};
    };
}

}
