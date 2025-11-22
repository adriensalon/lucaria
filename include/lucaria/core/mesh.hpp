#pragma once

#include <unordered_map>

#include <lucaria/core/geometry.hpp>

namespace lucaria {

/// @brief Represents the different available mesh attributes
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
    LUCARIA_DELETE_DEFAULT(mesh)
    mesh(const mesh& other) = delete;
    mesh& operator=(const mesh& other) = delete;
    mesh(mesh&& other);
    mesh& operator=(mesh&& other);
    ~mesh();

    /// @brief Creates a mesh from geometry data
    /// @param from the geometry data to create from
    mesh(const geometry& from);

    /// @brief Update a mesh attribute buffer from geometry data
    /// @param from the geometry data to update from
    /// @param attribute the selected attribute to use from the geometry data
    /// @param size the selected count to update
    /// @param offset the selected offset to update from
    void update_attribute(const geometry& from, const mesh_attribute attribute, const glm::uint size, const glm::uint offset = 0);

    /// @brief Updates the mesh indices buffer from geometry data
    /// @param from the geometry data to update from
    /// @param size the selected count to update
    /// @param offset the selected offset to update from
    void update_indices(const geometry& from, const glm::uint size, const glm::uint offset = 0);

    /// @brief Returns the mesh vertices count
    /// @return the vertices count
    [[nodiscard]] glm::uint get_size() const;

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_array_handle() const;

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_elements_handle() const;

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handles
    [[nodiscard]] const std::unordered_map<mesh_attribute, glm::uint>& get_attribute_handles() const;

    /// @brief Returns a handle to the CPU stored invposes matrices
    /// @return the CPU stored invposes matrices
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
[[nodiscard]] fetched<mesh> fetch_mesh(const std::filesystem::path& data_path);

// Internal definitions
namespace _detail {

#if LUCARIA_CONFIG_DEBUG

    struct guizmo_mesh {
        LUCARIA_DELETE_DEFAULT(guizmo_mesh)
        guizmo_mesh(const guizmo_mesh& other) = delete;
        guizmo_mesh& operator=(const guizmo_mesh& other) = delete;
        guizmo_mesh(guizmo_mesh&& other);
        guizmo_mesh& operator=(guizmo_mesh&& other);
        ~guizmo_mesh();

        guizmo_mesh(const geometry& from);
        guizmo_mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);

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

#endif

}
}
