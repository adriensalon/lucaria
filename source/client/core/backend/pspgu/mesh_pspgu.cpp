#include <lucaria/core/mesh.hpp>

namespace lucaria {
namespace detail {

    mesh_implementation::mesh_implementation(mesh_implementation&& other)
    {
    }

    mesh_implementation& mesh_implementation::operator=(mesh_implementation&& other)
    {
        return *this;
    }

    mesh_implementation::~mesh_implementation()
    {
    }

    mesh_implementation::mesh_implementation(const geometry_implementation& from)
    {		
		invposes = from.data.invposes;
        size = 3 * static_cast<glm::uint>(from.data.indices.size());
    }

}

namespace _detail {

#if defined(LUCARIA_DEBUG)
    guizmo_mesh::guizmo_mesh(guizmo_mesh&& other)
    {
    }

    guizmo_mesh& guizmo_mesh::operator=(guizmo_mesh&& other)
    {
        return *this;
    }

    guizmo_mesh::~guizmo_mesh()
    {
    }

    guizmo_mesh::guizmo_mesh(const detail::geometry_implementation& from)
    {
    }

    guizmo_mesh::guizmo_mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    void guizmo_mesh::update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    glm::uint guizmo_mesh::get_size() const
    {
        return _size;
    }

    glm::uint guizmo_mesh::get_array_handle() const
    {
        return _array_handle;
    }

    glm::uint guizmo_mesh::get_elements_handle() const
    {
        return _elements_handle;
    }

    glm::uint guizmo_mesh::get_positions_handle() const
    {
        return _positions_handle;
    }
#endif
}
}