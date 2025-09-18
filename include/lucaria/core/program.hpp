#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <sstream>

#include <glm/glm.hpp>

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/common/shader.hpp>

struct program_ref {
    program_ref() = delete;
    program_ref(const program_ref& other) = delete;
    program_ref& operator=(const program_ref& other) = delete;
    program_ref(program_ref&& other);
    program_ref& operator=(program_ref&& other);
    ~program_ref();

    program_ref(const shader_data& vertex, const shader_data& fragment);
    void use() const;
    void bind(const std::string& name, const mesh_ref& mesh, const mesh_attribute attribute);
    void bind(const std::string& name, const cubemap_ref& cubemap, const glm::uint slot = 0) const;
    void bind(const std::string& name, const texture_ref& texture, const glm::uint slot = 0) const;
    template <typename value_t> void bind(const std::string& name, const value_t& value);
    void draw(const bool use_depth = true) const;
    glm::uint get_id() const;
#if LUCARIA_GUIZMO
    void bind_guizmo(const std::string& name, const guizmo_mesh_ref& mesh);
    void draw_guizmo() const;
#endif

private:
    bool _is_instanced;
    glm::uint _program_id;
    glm::uint _array_id;
    glm::uint _indices_count;
    std::unordered_map<std::string, glm::int32> _program_attributes;
    std::unordered_map<std::string, glm::int32> _program_uniforms;
};

shader_data load_shader_data(const std::vector<char>& shader_bytes);
std::future<std::shared_ptr<program_ref>> fetch_program(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path);
void clear_program_fetches();
