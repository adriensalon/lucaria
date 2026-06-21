#include <lucaria/core/mesh.hpp>

namespace lucaria {
namespace detail {

    asset_mesh::asset_mesh(asset_mesh&& other)
    {
    }

    asset_mesh& asset_mesh::operator=(asset_mesh&& other)
    {
        return *this;
    }

    asset_mesh::~asset_mesh()
    {
    }

    asset_mesh::asset_mesh(const object_geometry& from)
    {		
		invposes = from.data.invposes;
        size = 3 * static_cast<glm::uint>(from.data.indices.size());
    }

}

namespace _detail {

#if defined(LUCARIA_DEBUG)
    rendering_mesh_line::rendering_mesh_line(rendering_mesh_line&& other)
    {
    }

    rendering_mesh_line& rendering_mesh_line::operator=(rendering_mesh_line&& other)
    {
        return *this;
    }

    rendering_mesh_line::~rendering_mesh_line()
    {
    }

    rendering_mesh_line::rendering_mesh_line(const detail::object_geometry& from)
    {
    }

    rendering_mesh_line::rendering_mesh_line(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    void rendering_mesh_line::update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
    }

    glm::uint rendering_mesh_line::get_size() const
    {
        return _size;
    }

    glm::uint rendering_mesh_line::get_array_handle() const
    {
        return _array_handle;
    }

    glm::uint rendering_mesh_line::get_elements_handle() const
    {
        return _elements_handle;
    }

    glm::uint rendering_mesh_line::get_positions_handle() const
    {
        return _positions_handle;
    }
#endif
}
}