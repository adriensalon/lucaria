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
    /// @param aabb 
    volume_data(const std::pair<glm::vec3, glm::vec3>& aabb);

    /// @brief 
    /// @param aabbs 
    volume_data(const std::vector<std::pair<glm::vec3, glm::vec3>>& aabbs);

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
bool get_is_contained(const glm::vec3& point);

/// @brief 
/// @param axis 
/// @param point 
/// @return 
std::optional<float> get_distance(const glm::vec3& axis, const glm::vec3& point);

/// @brief 
/// @param files 
/// @return 
volume_data load_volume(const std::vector<std::filesystem::path>& files);

/// @brief 
/// @param file 
/// @return 
std::future<volume_data> fetch_volume(const std::vector<std::filesystem::path>& files);