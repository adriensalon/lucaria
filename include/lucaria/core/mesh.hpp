#pragma once

#include <lucaria/bin/math_data.hpp>
#include <lucaria/core/geometry.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/refcount.hpp>
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

    enum struct mesh_origin {
        path,
        data
    };

    struct mesh_implementation {
        LUCARIA_DELETE_DEFAULT(mesh_implementation)
        mesh_implementation(const mesh_implementation& other) = delete;
        mesh_implementation& operator=(const mesh_implementation& other) = delete;
        mesh_implementation(mesh_implementation&& other) = default;
        mesh_implementation& operator=(mesh_implementation&& other) = default;
        ~mesh_implementation();

        mesh_implementation(const geometry_implementation& geometry);

        mesh_origin origin;

#if defined(LUCARIA_BACKEND_OPENGL)
        mesh_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        mesh_implementation_pspgu implementation_pspgu;
#endif

        std::vector<float32x4x4> invposes;
        uint32 size;
    };

    struct mesh_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct mesh_data_recipe {
        geometry_data data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using mesh_recipe = std::variant<mesh_path_recipe, mesh_data_recipe>;

	[[nodiscard]] mesh_recipe make_recipe(const implementation_container<mesh_implementation>& container);
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
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::mesh_implementation>* _manager = nullptr;
    detail::implementation_container<detail::mesh_implementation>* _resource = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct detail::rendering_system;
	friend class cereal::access;
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
