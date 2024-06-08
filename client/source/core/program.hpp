#pragma once

#include <filesystem>

#include <GLES3/gl3.h>

#include <core/cubemap.hpp>
#include <core/mesh.hpp>
#include <core/texture.hpp>
#include <data/shader.hpp>

/// @brief Represents a GPU program managed by the application
struct program_ref {

    /// @brief Default constructor is not allowed because this object must be created from data
    program_ref() = delete;

    /// @brief Loads the program from an asset data
    /// @param vertex
    /// @param fragment
    program_ref(const shader_data& vertex, const shader_data& fragment);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other
    program_ref(const program_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other
    /// @return
    program_ref& operator=(const program_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other
    program_ref(program_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other
    /// @return
    program_ref& operator=(program_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~program_ref();

    void use() const;

    void bind(const cubemap_ref& cubemap, const GLuint slot = 0) const;

    void bind(const std::vector<cubemap_ref>& cubemaps, const GLuint slot = 0) const;
    
    void bind(const mesh_ref& mesh) const;

    void bind(const texture_ref& texture, const GLuint slot = 0) const;

    void bind(const std::vector<texture_ref>& textures, const GLuint slot = 0) const;

    template <typename value_t>
    void bind(const std::string& name, const value_t& value);

    void draw() const;

    /// @brief Gets the OpenGL id for this managed data
    /// @return the program id as a GLuint
    GLuint get_id() const;

private:
    GLuint _program_id;
    GLuint _vertices_count = 0;
    std::unordered_map<std::string, GLuint> _program_attributes;
    std::unordered_map<std::string, GLuint> _program_uniforms;
    inline static GLuint _used_program_id = 0;
};

/// @brief
/// @param file
/// @return
shader_data load_shader(const std::filesystem::path& file);
