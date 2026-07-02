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
        void bind_attribute(const std::string& name, const rendering_mesh& mesh, const data_vertex_attribute attribute);
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
        uint32 bound_index_offset = 0;
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
        flag_owning ownership = {};
        VkShaderModule vertex_module = VK_NULL_HANDLE;
        VkShaderModule fragment_module = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        VkDescriptorSetLayout bones_descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorSet bones_descriptor_set = VK_NULL_HANDLE;
        VkBuffer bones_buffer = VK_NULL_HANDLE;
        VkDeviceMemory bones_memory = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkFormat pipeline_color_format = VK_FORMAT_UNDEFINED;
        VkFormat pipeline_depth_format = VK_FORMAT_UNDEFINED;
        VkPrimitiveTopology pipeline_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        bool pipeline_uses_texture = false;
        bool pipeline_uses_depth = true;
        uint32 pipeline_vertex_stride = 0;
        uint32 pipeline_position_offset = 0;
        uint32 pipeline_texcoord_offset = 0;
        uint32 pipeline_normal_offset = 0;
        uint32 pipeline_bones_offset = 0;
        uint32 pipeline_weights_offset = 0;
        bool shader_uses_texture = false;
        bool shader_uses_cubemap = false;
        bool shader_uses_bones = false;
        const rendering_mesh* bound_mesh = nullptr;
        const rendering_mesh_line* bound_line_mesh = nullptr;
        uint32 bound_indices_count = 0;
        uint32 bound_index_offset = 0;
        mutable const rendering_texture* bound_texture = nullptr;
        mutable const rendering_cubemap* bound_cubemap = nullptr;
        mutable float32x4x4 push_matrix = float32x4x4(1.f);
        mutable float32x4 push_color = { 1.f, 1.f, 1.f, 1.f };
        mutable float32x4 push_uv_rect = { 0.f, 0.f, 1.f, 1.f };
        mutable float32x4 push_texel_size_fxaa = { 0.f, 0.f, 0.f, 0.f };
        mutable float32x4 push_fxaa_params = { 0.f, 0.f, 0.f, 0.f };
        mutable bool push_use_texture = false;
        mutable bool push_has_texcoord = false;
        std::unordered_map<std::string, uint32> reflected_attributes = {};
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        flag_owning ownership = {};
        const rendering_mesh* mesh = nullptr;
        mutable const rendering_texture* texture = nullptr;
        mutable bool texture_enabled = false;
        bool lighting_enabled = false;
        bool depth_enabled = true;
        mutable bool transform_bound = false;
#endif
    };

}
}
