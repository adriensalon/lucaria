#pragma once

#include <unordered_map>

#include <lucaria/core/geometry.hpp>

namespace lucaria {

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

/// @brief Represents runtime geometry on the device
struct mesh {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(mesh)
    mesh(const mesh& other) = delete;
    mesh& operator=(const mesh& other) = delete;
    mesh(mesh&& other);
    mesh& operator=(mesh&& other);
    ~mesh();

    /// @brief
    /// @param from
    mesh(const geometry& from);

    /// @brief
    /// @param from
    /// @param attribute
    /// @param size
    /// @param offset
    void update_attribute(const geometry& from, const mesh_attribute attribute, const glm::uint size, const glm::uint offset = 0);

    /// @brief
    /// @param from
    /// @param size
    /// @param offset
    void update_indices(const geometry& from, const glm::uint size, const glm::uint offset = 0);

    [[nodiscard]] glm::uint get_size() const;
    [[nodiscard]] glm::uint get_array_handle() const;
    [[nodiscard]] glm::uint get_elements_handle() const;
    [[nodiscard]] const std::unordered_map<mesh_attribute, glm::uint>& get_attribute_handles() const;
    [[nodiscard]] const std::vector<glm::mat4>& get_invposes() const;

private:
    bool _is_owning;
    glm::uint _size;
    glm::uint _array_handle;
    glm::uint _elements_handle;
    std::unordered_map<mesh_attribute, glm::uint> _attribute_handles;
    std::vector<glm::mat4> _invposes;
};

/// @brief Loads geometry from a file asynchronously and uploads directly to the device
/// @param data_path path to load from
[[nodiscard]] fetched<mesh> fetch_mesh(const std::filesystem::path& geometry_data_path);

struct guizmo_mesh {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(guizmo_mesh)
    guizmo_mesh(const guizmo_mesh& other) = delete;
    guizmo_mesh& operator=(const guizmo_mesh& other) = delete;
    guizmo_mesh(guizmo_mesh&& other);
    guizmo_mesh& operator=(guizmo_mesh&& other);
    ~guizmo_mesh();

    /// @brief
    /// @param from
    guizmo_mesh(const geometry& from);
    
    /// @brief
    /// @param positions
    /// @param indices
    guizmo_mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);

    /// @brief
    /// @param positions
    void update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
    
    [[nodiscard]] glm::uint get_size() const;
    [[nodiscard]] glm::uint get_array_handle() const;
    [[nodiscard]] glm::uint get_elements_handle() const;
    [[nodiscard]] glm::uint get_positions_handle() const;

private:
    bool _is_owning;
    glm::uint _size;
    glm::uint _array_handle;
    glm::uint _elements_handle;
    glm::uint _positions_handle;
};

namespace detail {

    inline const std::unordered_map<mesh_attribute, glm::uint> mesh_attribute_sizes = {
        { mesh_attribute::position, 3 },
        { mesh_attribute::color, 3 },
        { mesh_attribute::normal, 3 },
        { mesh_attribute::tangent, 3 },
        { mesh_attribute::bitangent, 3 },
        { mesh_attribute::texcoord, 2 },
        { mesh_attribute::bones, 4 },
        { mesh_attribute::weights, 4 },
    };
}
}
