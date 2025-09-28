#pragma once

#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/geometry.hpp>

namespace lucaria {

/// @brief Represents an uploaded viewport on the device
struct viewport {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(viewport)
    viewport(const viewport& other) = delete;
    viewport& operator=(const viewport& other) = delete;
    viewport(viewport&& other);
    viewport& operator=(viewport&& other);
    ~viewport();

    /// @brief
    /// @param from
    viewport(const geometry& from, const glm::float32 pixels_per_meter);

    [[nodiscard]] glm::uvec2 get_computed_screen_size() const;
    [[nodiscard]] glm::uint get_size() const;
    [[nodiscard]] glm::uint get_array_handle() const;
    [[nodiscard]] glm::uint get_elements_handle() const;
    [[nodiscard]] glm::uint get_positions_handle() const;
    [[nodiscard]] glm::uint get_texcoords_handle() const;

private:
    bool _is_owning;
    glm::uvec2 _computed_framebuffer_size;
    glm::uint _size;
    glm::uint _array_handle;
    glm::uint _elements_handle;
    glm::uint _positions_handle;
    glm::uint _texcoords_handle;
};

/// @brief Loads a viewport from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<viewport> fetch_viewport(const std::filesystem::path& data_path, const glm::float32 pixels_per_meter);

}
