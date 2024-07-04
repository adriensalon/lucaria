#pragma once

#include <filesystem>
#include <future>

#include <GLES3/gl3.h>

#include <data/texture.hpp>

/// @brief Represents a GPU texture managed by the application
struct texture_ref {
    texture_ref() = delete;
    texture_ref(const texture_ref& other) = delete;
    texture_ref& operator=(const texture_ref& other) = delete;
    texture_ref(texture_ref&& other);
    texture_ref& operator=(texture_ref&& other);
    ~texture_ref();

    /// @brief Loads the texture from an asset data
    /// @param file the binary asset data to load from
    texture_ref(const texture_data& data);
    
    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_id() const;

private:
    GLuint _texture_id;
    bool _must_destroy;
};

/// @brief 
/// @param file 
/// @return 
texture_ref load_texture(const std::filesystem::path& file);

/// @brief 
/// @param file 
/// @return 
std::future<texture_ref> fetch_texture(const std::filesystem::path& file);