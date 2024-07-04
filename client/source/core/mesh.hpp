#pragma once

#include <filesystem>
#include <future>
#include <unordered_map>
#include <vector>

#include <GLES3/gl3.h>

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

inline const std::unordered_map<mesh_attribute, GLuint> mesh_attribute_sizes = {
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
    mesh_ref(const mesh_data& data);

    /// @brief 
    /// @return 
    std::unordered_map<mesh_attribute, GLuint> get_buffer_ids() const;

    /// @brief Destroys an existing buffer with this attribute and recreate it with new data.
    /// @param attribute is the selected attribute.
    /// @param data is the new data to upload to GPU.
    void update_buffer(const mesh_attribute attribute, const std::vector<float>& data);

    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_array_id() const;

    /// @brief 
    /// @return 
    GLuint get_count() const;

private:
    GLuint _count;
    GLuint _array_id;
    GLuint _elements_id;
    std::unordered_map<mesh_attribute, GLuint> _attribute_ids;
    bool _must_destroy;
};

/// @brief 
/// @param file 
/// @return
mesh_ref load_mesh(const std::filesystem::path& file);

/// @brief 
/// @param file 
/// @return 
std::future<mesh_ref> fetch_mesh(const std::filesystem::path& file);

///
// std::future<std::vector<mesh_data>> fetch_meshes(const std::vector<std::filesystem::path>& files);
