#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/shader.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {

/// @brief Represents a runtime program on the device
struct program {
    LUCARIA_DELETE_DEFAULT(program)
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

    /// @brief Uses a mesh attribute for draw calls
    /// @param name source name of the attribute
    /// @param from fetched mesh to bind from
    /// @param attribute attribute type to bind
    void bind_attribute(const std::string& name, const fetched<mesh>& from, const mesh_attribute attribute);

    /// @brief Uses a cubemap uniform for draw calls
    /// @param name source name of the uniform
    /// @param from cubemap to bind from
    /// @param slot texture slot to use
    void bind_uniform(const std::string& name, const cubemap& from, const glm::uint slot = 0) const;

    /// @brief Uses a texture uniform for draw calls
    /// @param name source name of the uniform
    /// @param from texture to bind from
    /// @param slot texture slot to use
    void bind_uniform(const std::string& name, const texture& from, const glm::uint slot = 0) const;

    /// @brief Uses a uniform buffer for draw calls
    /// @tparam value_t type of the uniform data
    /// @param name source name of the uniform
    /// @param value data to bind from
    template <typename T>
    void bind_uniform(const std::string& name, const T& value);

    /// @brief Enqueues a draw call using all the bound resources
    /// @param use_depth enables reads and writes to the depth buffer
    void draw(const bool use_depth = true) const;

#if LUCARIA_CONFIG_DEBUG

    /// @brief
    /// @param name
    /// @param mesh
    void bind_guizmo(const std::string& name, const _detail::guizmo_mesh& from);

    /// @brief
    void draw_guizmo() const;
    
#endif

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;

    /// @brief Returns reflected attribute names from the compiled shaders
    /// @return the reflected attribute names
    [[nodiscard]] const std::unordered_map<std::string, glm::int32>& get_attributes() const;

    /// @brief Returns reflected uniform names from the compiled shaders
    /// @return the reflected uniform names
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
