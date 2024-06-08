#pragma once

#include <filesystem>
#include <vector>
#include <GLES3/gl3.h>

namespace lucaria {

/// @brief Represents a GPU texture managed by the program
struct texture {

    /// @brief Default constructor is not allowed because this object must be created from data
    texture() = delete;

    /// @brief Loads the texture from an asset file
    /// @param file the binary texture file to load from
    texture(const std::filesystem::path& file);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other 
    texture(const texture& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other 
    /// @return 
    texture& operator=(const texture& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other 
    texture(texture&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other 
    /// @return 
    texture& operator=(texture&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~texture();

    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_id() const;

private:
    GLuint _texture_id;
};

}