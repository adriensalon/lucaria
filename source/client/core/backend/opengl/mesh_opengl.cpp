#include <set>

#include <lucaria/core/mesh.hpp>

namespace lucaria {
namespace detail {

    namespace {

        [[nodiscard]] static glm::uint create_vertex_array()
        {
            glm::uint _array_id;
            glGenVertexArrays(1, &_array_id);
            glBindVertexArray(_array_id);
            return _array_id;
        }

        [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::vec2>& attribute)
        {
            glm::uint _attribute_id;
            glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec2*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::vec3>& attribute)
        {
            glm::uint _attribute_id;
            glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec3*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::vec4>& attribute)
        {
            glm::uint _attribute_id;
            glm::float32* _attribute_ptr = reinterpret_cast<glm::float32*>(const_cast<glm::vec4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::float32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::uvec4>& attribute)
        {
            glm::uint _attribute_id;
            glm::uint* _attribute_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::uint) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static glm::uint create_attribute_buffer(const std::vector<glm::ivec4>& attribute)
        {
            glm::uint _attribute_id;
            glm::int32* _attribute_ptr = reinterpret_cast<glm::int32*>(const_cast<glm::ivec4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::int32) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static glm::uint create_elements_buffer(const std::vector<glm::uvec2>& indices)
        {
            glm::uint _elements_id;
            glm::uint* _indices_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec2*>(indices.data()));
            glGenBuffers(1, &_elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indices.size() * sizeof(glm::uint), _indices_ptr, GL_STATIC_DRAW);
            return _elements_id;
        }

        [[nodiscard]] static glm::uint create_elements_buffer(const std::vector<glm::uvec3>& indices)
        {
            glm::uint _elements_id;
            glm::uint* _indices_ptr = reinterpret_cast<glm::uint*>(const_cast<glm::uvec3*>(indices.data()));
            glGenBuffers(1, &_elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * indices.size() * sizeof(glm::uint), _indices_ptr, GL_STATIC_DRAW);
            return _elements_id;
        }

        [[nodiscard]] static std::vector<glm::uvec2> generate_line_indices(const std::vector<glm::uvec3>& triangle_indices)
        {
            std::set<std::pair<glm::uint, glm::uint>> _edges;
            for (const glm::uvec3& _triangle : triangle_indices) {
                _edges.insert(std::minmax(_triangle.x, _triangle.y));
                _edges.insert(std::minmax(_triangle.y, _triangle.z));
                _edges.insert(std::minmax(_triangle.z, _triangle.x));
            }
            std::vector<glm::uvec2> _line_indices;
            for (const auto& _edge : _edges) {
                _line_indices.emplace_back(_edge.first, _edge.second);
            }
            return _line_indices;
        }

    }

    mesh_implementation::mesh_implementation(mesh_implementation&& other)
    {
        implementation_opengl.is_owning = false;
        *this = std::move(other);
    }

    mesh_implementation& mesh_implementation::operator=(mesh_implementation&& other)
    {
        if (implementation_opengl.is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }

        implementation_opengl.is_owning = other.implementation_opengl.is_owning;
        implementation_opengl.array_id = other.implementation_opengl.array_id;
        implementation_opengl.elements_id = other.implementation_opengl.elements_id;
        implementation_opengl.attribute_ids = std::move(other.implementation_opengl.attribute_ids);
        invposes = std::move(other.invposes);
        size = other.size;

        other.implementation_opengl.is_owning = false;
        return *this;
    }

    mesh_implementation::~mesh_implementation()
    {
        if (implementation_opengl.is_owning) {
            glDeleteVertexArrays(1, &implementation_opengl.array_id);
            glDeleteBuffers(1, &implementation_opengl.elements_id);
            for (const std::pair<const mesh_attribute, glm::uint>& _pair : implementation_opengl.attribute_ids) {
                glDeleteBuffers(1, &_pair.second);
            }
        }
    }

    mesh_implementation::mesh_implementation(const geometry_implementation& from)
    {
        implementation_opengl.array_id = create_vertex_array();
        implementation_opengl.elements_id = create_elements_buffer(from.data.indices);

        if (!from.data.positions.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::position] = create_attribute_buffer(from.data.positions);
        }
        if (!from.data.colors.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::color] = create_attribute_buffer(from.data.colors);
        }
        if (!from.data.normals.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::normal] = create_attribute_buffer(from.data.normals);
        }
        if (!from.data.tangents.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::tangent] = create_attribute_buffer(from.data.tangents);
        }
        if (!from.data.bitangents.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::bitangent] = create_attribute_buffer(from.data.bitangents);
        }
        if (!from.data.texcoords.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::texcoord] = create_attribute_buffer(from.data.texcoords);
        }
        if (!from.data.bones.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::bones] = create_attribute_buffer(from.data.bones);
        }
        if (!from.data.weights.empty()) {
            implementation_opengl.attribute_ids[mesh_attribute::weights] = create_attribute_buffer(from.data.weights);
        }
		
		invposes = from.data.invposes;
        size = 3 * static_cast<glm::uint>(from.data.indices.size());

        implementation_opengl.is_owning = true;
    }

}

namespace _detail {

#if defined(LUCARIA_DEBUG)
    guizmo_mesh::guizmo_mesh(guizmo_mesh&& other)
    {
        *this = std::move(other);
    }

    guizmo_mesh& guizmo_mesh::operator=(guizmo_mesh&& other)
    {
        _is_owning = true;
        _size = other._size;
        _array_handle = other._array_handle;
        _elements_handle = other._elements_handle;
        _positions_handle = other._positions_handle;
        other._is_owning = false;
        return *this;
    }

    guizmo_mesh::~guizmo_mesh()
    {
        if (_is_owning) {
            glDeleteVertexArrays(1, &_array_handle);
            glDeleteBuffers(1, &_elements_handle);
            glDeleteBuffers(1, &_positions_handle);
        }
    }

    guizmo_mesh::guizmo_mesh(const detail::geometry_implementation& from)
    {
        const std::vector<glm::uvec2> _line_indices = detail::generate_line_indices(from.data.indices);
        _array_handle = detail::create_vertex_array();
        _size = 2 * static_cast<glm::uint>(_line_indices.size());
        _elements_handle = detail::create_elements_buffer(_line_indices);
        _positions_handle = detail::create_attribute_buffer(from.data.positions);
        _is_owning = true;
    }

    guizmo_mesh::guizmo_mesh(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
        _array_handle = detail::create_vertex_array();
        _size = 2 * static_cast<glm::uint>(indices.size());
        _elements_handle = detail::create_elements_buffer(indices);
        _positions_handle = detail::create_attribute_buffer(positions);
        _is_owning = true;
    }

    void guizmo_mesh::update(const std::vector<glm::vec3>& positions, const std::vector<glm::uvec2>& indices)
    {
        glBindVertexArray(_array_handle);
        glBindBuffer(GL_ARRAY_BUFFER, _positions_handle);
        glBufferData(GL_ARRAY_BUFFER, 3 * positions.size() * sizeof(glm::float32), positions.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indices.size() * sizeof(glm::uint), indices.data(), GL_STATIC_DRAW);
        _size = 2 * static_cast<glm::uint>(indices.size());
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