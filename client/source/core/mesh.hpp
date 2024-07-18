#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <future>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <data/mesh.hpp>

enum struct mesh_attribute {
    position,
    color,
    normal,
    tangent,
    bitangent,
    texcoord,
};

inline const std::unordered_map<mesh_attribute, std::size_t> mesh_attribute_sizes = {
    { mesh_attribute::position, 3 },
    { mesh_attribute::color, 3 },
    { mesh_attribute::normal, 3 },
    { mesh_attribute::tangent, 3 },
    { mesh_attribute::bitangent, 3 },
    { mesh_attribute::texcoord, 2 },
};

struct mesh_ref {
    mesh_ref() = delete;
    mesh_ref(const mesh_ref& other) = delete;
    mesh_ref& operator=(const mesh_ref& other) = delete;
    mesh_ref(mesh_ref&& other);
    mesh_ref& operator=(mesh_ref&& other);
    ~mesh_ref();

    mesh_ref(const mesh_data& data);
    void update_positions(const std::vector<glm::vec3>& new_positions);
    std::unordered_map<mesh_attribute, glm::uint> get_buffer_ids() const;
    glm::uint get_array_id() const;
    glm::uint get_count() const;

private:
    bool _is_instanced;
    glm::uint _count;
    glm::uint _array_id;
    glm::uint _elements_id;
    std::unordered_map<mesh_attribute, glm::uint> _attribute_ids;
};

mesh_data load_mesh_data(std::istringstream& mesh_stream);
std::shared_future<std::shared_ptr<mesh_ref>> fetch_mesh(const std::filesystem::path& mesh_path);

#if LUCARIA_GUIZMO

struct guizmo_mesh_ref {
    guizmo_mesh_ref() = delete;
    guizmo_mesh_ref(const guizmo_mesh_ref& other) = delete;
    guizmo_mesh_ref& operator=(const guizmo_mesh_ref& other) = delete;
    guizmo_mesh_ref(guizmo_mesh_ref&& other);
    guizmo_mesh_ref& operator=(guizmo_mesh_ref&& other);
    ~guizmo_mesh_ref();

    guizmo_mesh_ref(const mesh_data& data);
    glm::uint get_positions_id() const;
    glm::uint get_array_id() const;
    glm::uint get_count() const;

private:
    bool _is_instanced;
    glm::uint _count;
    glm::uint _array_id;
    glm::uint _elements_id;
    glm::uint _positions_id;
};

#endif