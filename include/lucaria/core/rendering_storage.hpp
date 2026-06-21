#pragma once

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <vector>

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/core/rendering_backend.hpp>

namespace lucaria {
namespace detail {

    struct rendering_mesh;

    constexpr uint32 rendering_invalid_page = static_cast<uint32>(-1);

    struct rendering_allocator_range {
        uint32 offset = 0;
        uint32 size = 0;
    };

    struct rendering_allocator_free_list {
        [[nodiscard]] std::optional<rendering_allocator_range> allocate(const uint32 size, const uint32 alignment);
        void free(const rendering_allocator_range range);

    private:
        std::vector<rendering_allocator_range> ranges = {};
    };

    struct rendering_mesh_allocation {
        uint32 page = rendering_invalid_page;
        rendering_allocator_range vertices = {};
        rendering_allocator_range elements = {};
    };

    struct rendering_meshes_page {
        uint32 vertex_capacity = 0;
        uint32 element_capacity = 0;
        uint32 allocations = 0;
        rendering_allocator_free_list vertices = {};
        rendering_allocator_free_list elements = {};

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint vertices_id = 0;
        GLuint elements_id = 0;
#endif
    };

    struct rendering_meshes_buffer {
        [[nodiscard]] std::optional<rendering_mesh_allocation> allocate(const uint32 vertex_size, const uint32 element_size, const uint32 vertex_alignment, const uint32 element_alignment);
        void free(const rendering_mesh_allocation allocation);

    private:
        data_geometry_profile profile = 0;
        std::vector<rendering_meshes_page> pages = {};
        friend struct rendering_meshes_registry;
    };

    struct rendering_meshes_registry {
        rendering_meshes_registry() = default;
        rendering_meshes_registry(const rendering_meshes_registry&) = delete;
        rendering_meshes_registry& operator=(const rendering_meshes_registry&) = delete;
        ~rendering_meshes_registry();

        [[nodiscard]] rendering_meshes_buffer& assure_pool(const data_geometry_profile profile);
        [[nodiscard]] rendering_meshes_buffer* find_pool(const data_geometry_profile profile);
        void upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements);
        void release(rendering_mesh& mesh);

    private:
        std::unordered_map<data_geometry_profile, rendering_meshes_buffer> pools = {};
    };

    [[nodiscard]] uint32 rendering_align_up(const uint32 value, const uint32 alignment);

}
}
