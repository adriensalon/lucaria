#pragma once

#include <filesystem>
#include <future>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <lucaria/common/geometry.hpp>

enum struct mesh_attribute {
    position,
    color,
    normal,
    tangent,
    bitangent,
    texcoord,
    bones,
    weights
};

inline const std::unordered_map<mesh_attribute, std::size_t> mesh_attribute_sizes = {
    { mesh_attribute::position, 3 },
    { mesh_attribute::color, 3 },
    { mesh_attribute::normal, 3 },
    { mesh_attribute::tangent, 3 },
    { mesh_attribute::bitangent, 3 },
    { mesh_attribute::texcoord, 2 },
    { mesh_attribute::bones, 4 },
    { mesh_attribute::weights, 4 },
};

struct mesh_ref {
    mesh_ref() = delete;
    mesh_ref(const mesh_ref& other) = delete;
    mesh_ref& operator=(const mesh_ref& other) = delete;
    mesh_ref(mesh_ref&& other);
    mesh_ref& operator=(mesh_ref&& other);
    ~mesh_ref();

    mesh_ref(const geometry_data& data);
    std::unordered_map<mesh_attribute, glm::uint> get_buffer_ids() const;
    glm::uint get_array_id() const;
    glm::uint get_indices_count() const;
    const std::vector<glm::mat4>& get_invposes() const;

private:
    bool _is_instanced;
    glm::uint _indices_count;
    glm::uint _array_id;
    glm::uint _elements_id;
    std::unordered_map<mesh_attribute, glm::uint> _attribute_ids;
    std::vector<glm::mat4> _invposes;
    friend struct program_ref;
};

geometry_data load_geometry_data(const std::vector<char>& geometry_bytes);
std::shared_future<std::shared_ptr<mesh_ref>> fetch_mesh(const std::filesystem::path& geometry_path);
void clear_mesh_fetches();

#if LUCARIA_GUIZMO

struct guizmo_mesh_ref {
    guizmo_mesh_ref() = delete;
    guizmo_mesh_ref(const guizmo_mesh_ref& other) = delete;
    guizmo_mesh_ref& operator=(const guizmo_mesh_ref& other) = delete;
    guizmo_mesh_ref(guizmo_mesh_ref&& other);
    guizmo_mesh_ref& operator=(guizmo_mesh_ref&& other);
    ~guizmo_mesh_ref();

    guizmo_mesh_ref(const geometry_data& data);
    guizmo_mesh_ref(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
    void update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
    glm::uint get_positions_id() const;
    glm::uint get_array_id() const;
    glm::uint get_indices_count() const;

private:
    bool _is_instanced;
    glm::uint _indices_count;
    glm::uint _array_id;
    glm::uint _elements_id;
    glm::uint _positions_id;
};

#endif