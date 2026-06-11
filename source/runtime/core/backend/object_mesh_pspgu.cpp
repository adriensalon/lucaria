#include <lucaria/core/mesh.hpp>

namespace lucaria {
namespace detail {

    object_mesh::object_mesh(object_mesh&& other)
    {
    }

    object_mesh& object_mesh::operator=(object_mesh&& other)
    {
        return *this;
    }

    object_mesh::~object_mesh()
    {
    }

    object_mesh::object_mesh(const object_geometry& from)
    {		
		invposes = from.data.invposes;
        size = 3 * static_cast<glm::uint>(from.data.indices.size());
    }

}

namespace _detail {

#if defined(LUCARIA_DEBUG)
    object_mesh_line::object_mesh_line(object_mesh_line&& other)
    {
    }

    object_mesh_line& object_mesh_line::operator=(object_mesh_line&& other)
    {
        return *this;
    }

    object_mesh_line::~object_mesh_line()
    {
    }

    object_mesh_line::object_mesh_line(const detail::object_geometry& from)
    {
    }

    object_mesh_line::object_mesh_line(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    void object_mesh_line::update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    glm::uint object_mesh_line::get_size() const
    {
        return _size;
    }

    glm::uint object_mesh_line::get_array_handle() const
    {
        return _array_handle;
    }

    glm::uint object_mesh_line::get_elements_handle() const
    {
        return _elements_handle;
    }

    glm::uint object_mesh_line::get_positions_handle() const
    {
        return _positions_handle;
    }
#endif
}
}