#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/rendering_backend.hpp>

namespace lucaria {
namespace detail {

    struct rendering_mesh;
    struct rendering_texture;

    constexpr uint32 rendering_invalid_page = static_cast<uint32>(-1);

    struct rendering_mesh_range {
        uint32 offset = 0;
        uint32 size = 0;
    };

    struct rendering_texture_range {
        uint32x2 offset = {};
        uint32x2 size = {};
    };

    struct rendering_mesh_allocation {
        uint32 page = rendering_invalid_page;
        rendering_mesh_range vertices = {};
        rendering_mesh_range elements = {};
    };

    struct rendering_texture_allocation {
        uint32 page = rendering_invalid_page;
        rendering_texture_range pixels = {};
    };

    struct rendering_mesh_free_list {
        [[nodiscard]] std::optional<rendering_mesh_range> allocate(const uint32 size, const uint32 alignment);
        void free(const rendering_mesh_range range);

    private:
        std::vector<rendering_mesh_range> ranges = {};
    };

    struct rendering_meshes_page {
        uint32 vertex_capacity = 0;
        uint32 element_capacity = 0;
        uint32 allocations = 0;
        rendering_mesh_free_list free_vertices = {};
        rendering_mesh_free_list free_elements = {};

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint vertices_id = 0;
        GLuint elements_id = 0;
#endif
    };

    struct rendering_textures_page {
        data_image_profile profile = data_image_profile::rgba8888;
        uint32x2 capacity = {};
        uint32 allocations = 0;
        std::vector<rendering_texture_range> free_pixels = {};

#if defined(LUCARIA_BACKEND_OPENGL)
        GLuint texture_id = 0;
#endif

        [[nodiscard]] std::optional<rendering_texture_allocation> allocate(const uint32x2 size);
        void free(const rendering_texture_allocation allocation);
    };

    struct rendering_meshes_buffer {
        [[nodiscard]] std::optional<rendering_mesh_allocation> allocate(const uint32 vertex_size, const uint32 element_size, const uint32 vertex_alignment, const uint32 element_alignment);
        void free(const rendering_mesh_allocation allocation);

    private:
        data_geometry_profile profile = 0;
        std::vector<rendering_meshes_page> pages = {};
        friend struct rendering_meshes_registry;
    };

    struct rendering_textures_buffer {
        [[nodiscard]] std::optional<rendering_texture_allocation> allocate(const uint32x2 size);
        void free(const rendering_texture_allocation allocation);

    private:
        data_image_profile profile = data_image_profile::rgba8888;
        std::vector<rendering_textures_page> pages = {};
        friend struct rendering_textures_registry;
    };

    struct rendering_meshes_registry {
        rendering_meshes_registry() = default;
        rendering_meshes_registry(const rendering_meshes_registry&) = delete;
        rendering_meshes_registry& operator=(const rendering_meshes_registry&) = delete;
        ~rendering_meshes_registry();

        [[nodiscard]] rendering_meshes_buffer& assure_buffer(const data_geometry_profile profile);
        [[nodiscard]] rendering_meshes_buffer* find_buffer(const data_geometry_profile profile);
        void upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements);
        void release(rendering_mesh& mesh);

    private:
        std::unordered_map<data_geometry_profile, rendering_meshes_buffer> buffers = {};
    };

    struct rendering_textures_registry {
        rendering_textures_registry() = default;
        rendering_textures_registry(const rendering_textures_registry&) = delete;
        rendering_textures_registry& operator=(const rendering_textures_registry&) = delete;
        ~rendering_textures_registry();

        [[nodiscard]] rendering_textures_buffer& assure_buffer(const data_image_profile profile);
        [[nodiscard]] rendering_textures_buffer* find_buffer(const data_image_profile profile);
        void upload(rendering_texture& texture, const data_image& image);
        void release(rendering_texture& texture);

    private:
        std::unordered_map<data_image_profile, rendering_textures_buffer> buffers = {};
    };

    [[nodiscard]] uint32 rendering_meshes_align_up(const uint32 value, const uint32 alignment);

}
}
