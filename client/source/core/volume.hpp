#pragma once

#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include <core/mesh.hpp>

/// @brief 
struct volume_data {

    /// @brief 
    volume_data() = delete;

    /// @brief 
    /// @param box 
    volume_data(const std::pair<glm::vec3, glm::vec3>& box);

    /// @brief 
    /// @param boxes 
    volume_data(const std::vector<std::pair<glm::vec3, glm::vec3>>& boxes);

    /// @brief 
    /// @param mesh 
    volume_data(const mesh_data& mesh);

    /// @brief 
    /// @param meshes 
    volume_data(const std::vector<mesh_data>& meshes);

    /// @brief 
    /// @param other 
    volume_data(const volume_data& other) = default;

    /// @brief 
    /// @param other 
    /// @return 
    volume_data& operator=(const volume_data& other) = default;
    
    /// @brief 
    /// @param other 
    volume_data(volume_data&& other) = default;

    /// @brief 
    /// @param other 
    /// @return 
    volume_data& operator=(volume_data&& other) = default;

    /// @brief 
    std::vector<std::pair<glm::vec3, glm::vec3>> aabbs;
};

/// @brief 
/// @param point 
/// @return 
bool get_is_contained(const volume_data& volume, const glm::vec3& position);

/// @brief 
/// @param files 
/// @return 
volume_data load_volume(const std::vector<std::filesystem::path>& files);

/// @brief 
/// @param file 
/// @return 
std::future<volume_data> fetch_volume(const std::vector<std::filesystem::path>& files);