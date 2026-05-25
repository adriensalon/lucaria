#include <set>

#include <lucaria/core/object_mesh.hpp>

namespace lucaria {
namespace detail {

    namespace {

        [[nodiscard]] static GLuint _create_vertex_array()
        {
            GLuint _array_id;
            glGenVertexArrays(1, &_array_id);
            glBindVertexArray(_array_id);
            return _array_id;
        }

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<float32x2>& attribute)
        {
            GLuint _attribute_id;
            GLfloat* _attribute_ptr = reinterpret_cast<GLfloat*>(const_cast<float32x2*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<float32x3>& attribute)
        {
            GLuint _attribute_id;
            GLfloat* _attribute_ptr = reinterpret_cast<GLfloat*>(const_cast<float32x3*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<float32x4>& attribute)
        {
            GLuint _attribute_id;
            GLfloat* _attribute_ptr = reinterpret_cast<GLfloat*>(const_cast<float32x4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<uint32x4>& attribute)
        {
            GLuint _attribute_id;
            GLuint* _attribute_ptr = reinterpret_cast<GLuint*>(const_cast<uint32x4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLuint) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<int32x4>& attribute)
        {
            GLuint _attribute_id;
            GLint* _attribute_ptr = reinterpret_cast<GLint*>(const_cast<int32x4*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLint) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

        [[nodiscard]] static GLuint _create_elements_buffer(const std::vector<uint32x2>& indices)
        {
            GLuint _elements_id;
            GLuint* _indices_ptr = reinterpret_cast<GLuint*>(const_cast<uint32x2*>(indices.data()));
            glGenBuffers(1, &_elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(GLuint) * indices.size(), _indices_ptr, GL_STATIC_DRAW);
            return _elements_id;
        }

        [[nodiscard]] static GLuint _create_elements_buffer(const std::vector<uint32x3>& indices)
        {
            GLuint _elements_id;
            GLuint* _indices_ptr = reinterpret_cast<GLuint*>(const_cast<uint32x3*>(indices.data()));
            glGenBuffers(1, &_elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(GLuint) * indices.size(), _indices_ptr, GL_STATIC_DRAW);
            return _elements_id;
        }

        [[nodiscard]] static std::vector<uint32x2> _generate_line_indices(const std::vector<uint32x3>& triangle_indices)
        {
            std::set<std::pair<GLuint, GLuint>> _edges;
            for (const uint32x3& _triangle : triangle_indices) {
                _edges.insert(std::minmax(_triangle.x, _triangle.y));
                _edges.insert(std::minmax(_triangle.y, _triangle.z));
                _edges.insert(std::minmax(_triangle.z, _triangle.x));
            }

            std::vector<uint32x2> _line_indices;
            for (const std::pair<GLuint, GLuint>& _edge : _edges) {
                _line_indices.emplace_back(_edge.first, _edge.second);
            }

            return _line_indices;
        }

    }

    object_mesh::~object_mesh()
    {
        if (ownership.owns()) {
            glDeleteVertexArrays(1, &array_id);
            glDeleteBuffers(1, &elements_id);
            for (const std::pair<const object_mesh_attribute, GLuint>& _pair : attribute_ids) {
                glDeleteBuffers(1, &_pair.second);
            }
        }
    }

    object_mesh::object_mesh(const object_geometry& from)
        : origin(from.origin == object_geometry_origin::path ? object_mesh_origin::path : object_mesh_origin::data)
    {
        array_id = _create_vertex_array();
        elements_id = _create_elements_buffer(from.data.indices);

        if (!from.data.positions.empty()) {
            attribute_ids[object_mesh_attribute::position] = _create_attribute_buffer(from.data.positions);
        }
        if (!from.data.colors.empty()) {
            attribute_ids[object_mesh_attribute::color] = _create_attribute_buffer(from.data.colors);
        }
        if (!from.data.normals.empty()) {
            attribute_ids[object_mesh_attribute::normal] = _create_attribute_buffer(from.data.normals);
        }
        if (!from.data.tangents.empty()) {
            attribute_ids[object_mesh_attribute::tangent] = _create_attribute_buffer(from.data.tangents);
        }
        if (!from.data.bitangents.empty()) {
            attribute_ids[object_mesh_attribute::bitangent] = _create_attribute_buffer(from.data.bitangents);
        }
        if (!from.data.texcoords.empty()) {
            attribute_ids[object_mesh_attribute::texcoord] = _create_attribute_buffer(from.data.texcoords);
        }
        if (!from.data.bones.empty()) {
            attribute_ids[object_mesh_attribute::bones] = _create_attribute_buffer(from.data.bones);
        }
        if (!from.data.weights.empty()) {
            attribute_ids[object_mesh_attribute::weights] = _create_attribute_buffer(from.data.weights);
        }

        invposes = from.data.invposes;
        size = 3 * static_cast<uint32>(from.data.indices.size());

        ownership.emplace();
    }

    object_mesh_line::~object_mesh_line()
    {
        if (ownership.owns()) {
            glDeleteVertexArrays(1, &array_handle);
            glDeleteBuffers(1, &elements_handle);
            glDeleteBuffers(1, &positions_handle);
        }
    }

    object_mesh_line::object_mesh_line(const object_geometry& from)
    {
        const std::vector<uint32x2> _line_indices = _generate_line_indices(from.data.indices);
        array_handle = _create_vertex_array();
        size = 2 * static_cast<GLuint>(_line_indices.size());
        elements_handle = _create_elements_buffer(_line_indices);
        positions_handle = _create_attribute_buffer(from.data.positions);
        ownership.emplace();
    }

    object_mesh_line::object_mesh_line(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
    {
        array_handle = _create_vertex_array();
        size = 2 * static_cast<GLuint>(indices.size());
        elements_handle = _create_elements_buffer(indices);
        positions_handle = _create_attribute_buffer(positions);
        ownership.emplace();
    }

    void object_mesh_line::update(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
    {
        glBindVertexArray(array_handle);
        glBindBuffer(GL_ARRAY_BUFFER, positions_handle);
        glBufferData(GL_ARRAY_BUFFER, 3 * positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        size = 2 * static_cast<GLuint>(indices.size());
    }
	
}
}