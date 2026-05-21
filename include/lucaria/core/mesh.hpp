#pragma once

#include <lucaria/core/geometry.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/resource.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/mesh_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/mesh_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

    struct rendering_system;

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

    struct mesh_implementation {
        LUCARIA_DELETE_DEFAULT(mesh_implementation)
        mesh_implementation(const mesh_implementation& other) = delete;
        mesh_implementation& operator=(const mesh_implementation& other) = delete;
        mesh_implementation(mesh_implementation&& other) = default;
        mesh_implementation& operator=(mesh_implementation&& other) = default;
        ~mesh_implementation();

        mesh_implementation(const geometry_implementation& geometry);

#if defined(LUCARIA_BACKEND_OPENGL)
        mesh_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        mesh_implementation_pspgu implementation_pspgu;
#endif

        std::vector<float32x4x4> invposes;
        uint32 size;
    };

}

struct mesh_object {
    mesh_object() = default;
    mesh_object(const mesh_object& other) = default;
    mesh_object& operator=(const mesh_object& other) = default;
    mesh_object(mesh_object&& other) = default;
    mesh_object& operator=(mesh_object&& other) = default;

    /// TODO GO CONTEXT
    static mesh_object create(const geometry_object geometry);

    /// TODO GO CONTEXT
    static mesh_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the mesh is ready to be used
    /// @return true if the mesh is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::resource_container<detail::mesh_implementation>* _resource = nullptr;
    explicit mesh_object(detail::resource_container<detail::mesh_implementation>* resource);
    friend struct detail::rendering_system;
};

// Internal definitions
namespace _detail {

#if defined(LUCARIA_DEBUG)

    struct guizmo_mesh {
        LUCARIA_DELETE_DEFAULT(guizmo_mesh)
        guizmo_mesh(const guizmo_mesh& other) = delete;
        guizmo_mesh& operator=(const guizmo_mesh& other) = delete;
        guizmo_mesh(guizmo_mesh&& other) = default;
        guizmo_mesh& operator=(guizmo_mesh&& other) = default;
        ~guizmo_mesh();

        guizmo_mesh(const detail::geometry_implementation& from);
        guizmo_mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
        void update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices);
        [[nodiscard]] glm::uint get_size() const;
        [[nodiscard]] glm::uint get_array_handle() const;
        [[nodiscard]] glm::uint get_elements_handle() const;
        [[nodiscard]] glm::uint get_positions_handle() const;

    private:
        detail::owning_flag _ownership = {};
        glm::uint _size;
        glm::uint _array_handle;
        glm::uint _elements_handle;
        glm::uint _positions_handle;
    };

#endif

}
}
