#pragma once

#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/geometry.hpp>

namespace lucaria {

/// @brief Represents an uploaded viewport on the device
struct viewport {
    LUCARIA_DELETE_DEFAULT(viewport)
    viewport(const viewport& other) = delete;
    viewport& operator=(const viewport& other) = delete;
    viewport(viewport&& other);
    viewport& operator=(viewport&& other);
    ~viewport();

    /// @brief Creates a viewport from geometry data and a fixed framebuffer size
    /// @param from the geometry data to create from
    /// @param size_pixels the fixed size to use for framebuffer size
    viewport(const geometry& from, const glm::uvec2& size_pixels);

    /// @brief Creates a viewport from geometry data and a ppm ratio
    /// @param from the geometry data to create from
    /// @param pixels_per_meter ppm ratio to use for computing framebuffer size
    viewport(const geometry& from, const glm::float32 pixels_per_meter);

    /// @brief Performs a raycast on CPU on the viewport mesh returning position in UV space
    /// @param view the view matrix to raycast from
    /// @return position in UV space
    [[nodiscard]] std::optional<glm::vec2> raycast(const glm::mat4& view);

    /// @brief Returns the computed or fixed framebuffer size
    /// @return the framebuffer size
    [[nodiscard]] glm::uvec2 get_computed_screen_size() const;
    
    /// @brief Returns the viewport vertices count
    /// @return the vertices count
    [[nodiscard]] glm::uint get_size() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_array_handle() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_elements_handle() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_positions_handle() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_texcoords_handle() const;

private:
    bool _is_owning;
    glm::uint _size;
    std::vector<glm::vec3> _positions;
    std::vector<glm::vec2> _texcoords;
    std::vector<glm::uvec3> _indices;
    glm::uvec2 _computed_framebuffer_size;
    glm::uint _array_handle;
    glm::uint _elements_handle;
    glm::uint _positions_handle;
    glm::uint _texcoords_handle;
};

/// @brief Loads a viewport from a file asynchronously
/// @param data_path path to load from
/// @param size_pixels the fixed size to use for framebuffer size
[[nodiscard]] fetched<viewport> fetch_viewport(const std::filesystem::path& data_path, const glm::uvec2& size_pixels);

/// @brief Loads a viewport from a file asynchronously
/// @param data_path path to load from
/// @param pixels_per_meter ppm ratio to use for computing framebuffer size
[[nodiscard]] fetched<viewport> fetch_viewport(const std::filesystem::path& data_path, const glm::float32 pixels_per_meter);

}
