#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/shader.hpp>
#include <lucaria/core/viewport.hpp>

namespace lucaria {
    
/// @brief Represents a runtime program on the device
struct program {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(program)
    program(const program& other) = delete;
    program& operator=(const program& other) = delete;
    program(program&& other);
    program& operator=(program&& other);
    ~program();

    /// @brief Compiles a program from shaders
    /// @param vertex vertex shader to create from
    /// @param fragment fragment shader to create from
    program(const shader& vertex, const shader& fragment);
    
    /// @brief Uses the program for draw calls
    void use() const;
    
    /// @brief Uses a mesh attribute for draw calls
    /// @param name source name of the attribute
    /// @param from mesh to bind from 
    /// @param attribute attribute type to bind
    void bind_attribute(const std::string& name, const mesh& from, const mesh_attribute attribute);
    
    /// @brief 
    /// @param from 
    void bind_attribute(const std::string& name, viewport& from, const mesh_attribute attribute);
    
    /// @brief Uses a cubemap uniform for draw calls
    /// @param name source name of the uniform
    /// @param from cubemap to bind from
    /// @param slot texture slot to use
    void bind_uniform(const std::string& name, const cubemap& from, const glm::uint slot = 0) const;
    
    /// @brief 
    /// @param name 
    /// @param from 
    /// @param slot 
    void bind_uniform(const std::string& name, const texture& from, const glm::uint slot = 0) const;
    
    /// @brief 
    /// @tparam value_t 
    /// @param name 
    /// @param value 
    template <typename value_t> 
    void bind_uniform(const std::string& name, const value_t& value);
    
    /// @brief 
    /// @param use_depth 
    void draw(const bool use_depth = true) const;

#if LUCARIA_GUIZMO

    /// @brief 
    /// @param name 
    /// @param mesh 
    void bind_guizmo(const std::string& name, const guizmo_mesh& from);

    /// @brief 
    void draw_guizmo() const;
#endif

    [[nodiscard]] glm::uint get_handle() const;
    [[nodiscard]] const std::unordered_map<std::string, glm::int32>& get_attributes() const;
    [[nodiscard]] const std::unordered_map<std::string, glm::int32>& get_uniforms() const;

private:
    bool _is_owning;
    glm::uint _handle;
    std::unordered_map<std::string, glm::int32> _attributes;
    std::unordered_map<std::string, glm::int32> _uniforms;
    glm::uint _bound_array_id;
    glm::uint _bound_indices_count;
};

/// @brief Loads shaders from files asynchronously and compiles a program directly on the device
/// @param vertex_data_path path to load vertex shader from
/// @param fragment_data_path path to load fragment shader from
[[nodiscard]] fetched<program> fetch_program(const std::filesystem::path& vertex_data_path, const std::filesystem::path& fragment_data_path);

}
