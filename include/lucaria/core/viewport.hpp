#pragma once

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
    viewport(const geometry& from); // + resolution
    
    /// @brief 
    /// @param offset 
    /// @param from 
    void update_positions(const geometry& from, const glm::uint size, const glm::uint offset = 0);
    
    /// @brief 
    /// @param offset 
    /// @param from 
    void update_indices(const geometry& from, const glm::uint size, const glm::uint offset = 0);
    

    [[nodiscard]] const std::vector<glm::vec3>& get_positions() const;
    [[nodiscard]] const std::vector<glm::uvec3>& get_indices() const;

private:
    bool _is_owning;
    glm::uint size;
    glm::uint array_handle;
    glm::uint elements_handle;
    glm::uint positions_handle;
    glm::uint texcoords_handle;
    glm::uvec2 framebuffer_size;
    std::vector<glm::vec3> _positions;
    std::vector<glm::uvec3> _indices;
};

/// @brief Loads a viewport from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<viewport> fetch_viewport(const std::filesystem::path& data_path);

}
