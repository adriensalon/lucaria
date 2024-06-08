#pragma once

#include <filesystem>

#include <GLES3/gl3.h>

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

    /// @brief Gets the OpenGL id for this managed data
    /// @return the program id as a GLuint
    GLuint get_id() const;

private:
    GLuint _program_id;
};

/// @brief
/// @param file
/// @return
shader_data load_shader(const std::filesystem::path& file);
