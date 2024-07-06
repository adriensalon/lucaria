#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <data/mesh.hpp>

/// @brief 
enum struct mesh_attribute {
    position,
    color,
    normal,
    tangent,
    bitangent,
    texcoord,
};

inline const std::unordered_map<mesh_attribute, glm::uint> mesh_attribute_sizes = {
    { mesh_attribute::position, 3 },
    { mesh_attribute::color, 3 },
    { mesh_attribute::normal, 3 },
    { mesh_attribute::tangent, 3 },
    { mesh_attribute::bitangent, 3 },
    { mesh_attribute::texcoord, 2 },
};

/// @brief 
struct mesh_ref {
    mesh_ref() = delete;
    mesh_ref(const mesh_ref& other) = delete;
    mesh_ref& operator=(const mesh_ref& other) = delete;
    mesh_ref(mesh_ref&& other);
    mesh_ref& operator=(mesh_ref&& other);
    ~mesh_ref();

    /// @brief 
    /// @param data 
    mesh_ref(const mesh_data& data, const bool keep_animation_data = false);

    /// @brief 
    /// @return 
    std::unordered_map<mesh_attribute, glm::uint> get_buffer_ids() const;

    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a glm::uint
    glm::uint get_array_id() const;

    /// @brief 
    /// @return 
    glm::uint get_count() const;

    /// @brief 
    /// @return 
    const std::vector<glm::vec3>& get_positions() const;

    /// @brief 
    /// @return 
    const std::vector<glm::uvec4>& get_bones() const;

    /// @brief 
    /// @return 
    const std::vector<glm::vec4>& get_weights() const;

    /// @brief 
    /// @param callback  
    void update_skinned_positions(const std::function<void(std::vector<glm::vec3>&)>& callback);

private:
    glm::uint _count;
    glm::uint _array_id;
    glm::uint _elements_id;
    std::unordered_map<mesh_attribute, glm::uint> _attribute_ids;
    std::vector<glm::vec3> _skinned_positions;
    std::vector<glm::vec3> _positions;
    std::vector<glm::uvec4> _bones;
    std::vector<glm::vec4> _weights;
    bool _must_destroy;
};

/// @brief 
/// @param file 
/// @return
mesh_ref load_mesh(const std::filesystem::path& file, const bool keep_positions = false);

/// @brief 
/// @param file 
/// @return 
std::future<mesh_ref> fetch_mesh(const std::filesystem::path& file, const bool keep_positions = false);
