#pragma once

#include <filesystem>

#include <GLES3/gl3.h>

#include <data/texture.hpp>

struct cubemap_data {
    
};

/// @brief Represents a GPU cubemap managed by the application
struct cubemap_ref {

    /// @brief Default constructor is not allowed because this object must be created from data
    cubemap_ref() = delete;

    /// @brief Loads the textures from asset binaries
    /// @param plus_x the +X texture binary
    /// @param plus_y the +Y texture binary
    /// @param plus_z the +Z texture binary
    /// @param minus_x the -X texture binary
    /// @param minus_y the -Y texture binary
    /// @param minus_z the -Z texture binary
    cubemap_ref(
        const texture_data& plus_x,
        const texture_data& plus_y,
        const texture_data& plus_z,
        const texture_data& minus_x,
        const texture_data& minus_y,
        const texture_data& minus_z);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other the other managed object
    cubemap_ref(const cubemap_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other the other managed object
    /// @return the same object
    cubemap_ref& operator=(const cubemap_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other the other managed object
    cubemap_ref(cubemap_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other the other managed object
    /// @return the same object
    cubemap_ref& operator=(cubemap_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~cubemap_ref();

    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_id() const;

private:
    GLuint _cubemap_id;
};