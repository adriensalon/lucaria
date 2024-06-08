#pragma once

#include <filesystem>

#include <GLES3/gl3.h>

#include <data/texture.hpp>

/// @brief Represents a GPU texture managed by the application
struct texture_ref {

    /// @brief Default constructor is not allowed because this object must be created from data
    texture_ref() = delete;

    /// @brief Loads the texture from an asset data
    /// @param file the binary asset data to load from
    texture_ref(const texture_data& data);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other 
    texture_ref(const texture_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other 
    /// @return 
    texture_ref& operator=(const texture_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other 
    texture_ref(texture_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other 
    /// @return 
    texture_ref& operator=(texture_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~texture_ref();
    
    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_id() const;

private:
    GLuint _texture_id;
};

/// @brief 
/// @param file 
/// @return 
texture_data load_texture(const std::filesystem::path& file);