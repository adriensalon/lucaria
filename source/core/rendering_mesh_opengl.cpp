#include <algorithm>
#include <cstddef>
#include <cstring>
#include <optional>
#include <set>
#include <utility>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>

namespace lucaria {
namespace detail {

    namespace {

        struct _packed_vertex_layout {
            uint32 stride = 0;
            std::unordered_map<data_vertex_attribute, rendering_mesh_range> attributes = {};
        };

        [[nodiscard]] static GLuint _create_vertex_array()
        {
            GLuint _array_id;
            glGenVertexArrays(1, &_array_id);
            glBindVertexArray(_array_id);
            return _array_id;
        }

        [[nodiscard]] static uint32 _get_vertices_count(const data_geometry& from)
        {
            if (from.vertices_count != 0) {
                return from.vertices_count;
            }
            uint32 _count = 0;
            auto _read = [&](const auto& values) {
                if (!values.empty()) {
                    _count = static_cast<uint32>(values.size());
                }
            };
            _read(from.positions);
            _read(from.colors);
            _read(from.normals);
            _read(from.tangents);
            _read(from.bitangents);
            _read(from.texcoords);
            _read(from.bones);
            _read(from.weights);
            return _count;
        }

        [[nodiscard]] static uint32 _attribute_storage_size(const data_vertex_attribute attribute)
        {
            switch (attribute) {
            case data_vertex_attribute::position:
                return sizeof(float32x3);
            case data_vertex_attribute::color:
                return sizeof(float32x4);
            case data_vertex_attribute::normal:
                return sizeof(float32x3);
            case data_vertex_attribute::tangent:
                return sizeof(float32x3);
            case data_vertex_attribute::bitangent:
                return sizeof(float32x3);
            case data_vertex_attribute::texcoord:
                return sizeof(float32x2);
            case data_vertex_attribute::bones:
                return sizeof(int32x4);
            case data_vertex_attribute::weights:
                return sizeof(float32x4);
            }
            return 0;
        }

        [[nodiscard]] static _packed_vertex_layout _make_packed_vertex_layout(const data_geometry& from)
        {
            _packed_vertex_layout _layout = {};
            auto _append = [&](const data_vertex_attribute attribute) {
                const uint32 _size = _attribute_storage_size(attribute);
                _layout.attributes[attribute] = { _layout.stride, _size };
                _layout.stride += _size;
            };
            if (!from.positions.empty()) {
                _append(data_vertex_attribute::position);
            }
            if (!from.colors.empty()) {
                _append(data_vertex_attribute::color);
            }
            if (!from.normals.empty()) {
                _append(data_vertex_attribute::normal);
            }
            if (!from.tangents.empty()) {
                _append(data_vertex_attribute::tangent);
            }
            if (!from.bitangents.empty()) {
                _append(data_vertex_attribute::bitangent);
            }
            if (!from.texcoords.empty()) {
                _append(data_vertex_attribute::texcoord);
            }
            if (!from.bones.empty()) {
                _append(data_vertex_attribute::bones);
            }
            if (!from.weights.empty()) {
                _append(data_vertex_attribute::weights);
            }
            return _layout;
        }

        template <typename VectorType>
        static void _write_packed_attribute(std::vector<uint8>& packed, const _packed_vertex_layout& layout, const data_vertex_attribute attribute, const std::vector<VectorType>& values, const uint32 vertices_count)
        {
            if (values.empty()) {
                return;
            }
            const auto _iterator = layout.attributes.find(attribute);
            if (_iterator == layout.attributes.end()) {
                return;
            }
            const rendering_mesh_range& _attribute = _iterator->second;
            LUCARIA_DEBUG_ASSERT(values.size() == vertices_count, "Invalid mesh attribute size")
            for (uint32 _index = 0; _index < vertices_count; ++_index) {
                uint8* _destination = packed.data() + static_cast<std::size_t>(_index) * layout.stride + _attribute.offset;
                std::memcpy(_destination, &values[_index], sizeof(VectorType));
            }
        }

        [[nodiscard]] static std::vector<uint8> _pack_vertices(const data_geometry& from, const _packed_vertex_layout& layout)
        {
            std::vector<uint8> _packed;
            const uint32 _vertices_count = _get_vertices_count(from);
            _packed.resize(static_cast<std::size_t>(_vertices_count) * layout.stride);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::position, from.positions, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::color, from.colors, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::normal, from.normals, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::tangent, from.tangents, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::bitangent, from.bitangents, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::texcoord, from.texcoords, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::bones, from.bones, _vertices_count);
            _write_packed_attribute(_packed, layout, data_vertex_attribute::weights, from.weights, _vertices_count);
            return _packed;
        }

        [[nodiscard]] static std::vector<uint32> _pack_triangle_indices(const std::vector<uint32x3>& indices)
        {
            std::vector<uint32> _packed;
            _packed.reserve(indices.size() * 3);
            for (const uint32x3& _index : indices) {
                _packed.push_back(_index.x);
                _packed.push_back(_index.y);
                _packed.push_back(_index.z);
            }
            return _packed;
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

        [[nodiscard]] static GLuint _create_attribute_buffer(const std::vector<float32x3>& attribute)
        {
            GLuint _attribute_id;
            GLfloat* _attribute_ptr = reinterpret_cast<GLfloat*>(const_cast<float32x3*>(attribute.data()));
            glGenBuffers(1, &_attribute_id);
            glBindBuffer(GL_ARRAY_BUFFER, _attribute_id);
            glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat) * attribute.size(), _attribute_ptr, GL_STATIC_DRAW);
            return _attribute_id;
        }

    }

    rendering_mesh::~rendering_mesh()
    {
        if (_ownership.owns()) {
            LUCARIA_DEBUG_ASSERT(_registry, "Unset rendering_mesh::meshes_registry")
            _registry->release(*this);
            glDeleteVertexArrays(1, &array_id);
        }
    }

    rendering_mesh::rendering_mesh(rendering_meshes_registry& registry, const data_geometry& from)
        : _registry(&registry)
    {
        array_id = _create_vertex_array();
        const _packed_vertex_layout _layout = _make_packed_vertex_layout(from);
        const std::vector<uint8> _packed_vertices = _pack_vertices(from, _layout);
        const std::vector<uint32> _packed_indices = _pack_triangle_indices(from.indices);
        profile = from.profile;
        vertex_stride = _layout.stride;
        for (const std::pair<const data_vertex_attribute, rendering_mesh_range>& _pair : _layout.attributes) {
            attribute_offsets[_pair.first] = _pair.second.offset;
        }
        _registry->upload(*this, _packed_vertices, _packed_indices);
        glBindVertexArray(array_id);
        glBindBuffer(GL_ARRAY_BUFFER, vertices_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_id);
        invposes = from.invposes;
        size = static_cast<uint32>(_packed_indices.size());
        _ownership.emplace();
    }

    rendering_mesh_line::~rendering_mesh_line()
    {
        if (_ownership.owns()) {
            glDeleteVertexArrays(1, &array_handle);
            glDeleteBuffers(1, &elements_handle);
            glDeleteBuffers(1, &positions_handle);
        }
    }

    rendering_mesh_line::rendering_mesh_line(const data_geometry& from)
    {
        const std::vector<uint32x2> _line_indices = _generate_line_indices(from.indices);
        array_handle = _create_vertex_array();
        size = 2 * static_cast<GLuint>(_line_indices.size());
        elements_handle = _create_elements_buffer(_line_indices);
        positions_handle = _create_attribute_buffer(from.positions);
        _ownership.emplace();
    }

    rendering_mesh_line::rendering_mesh_line(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
    {
        array_handle = _create_vertex_array();
        size = 2 * static_cast<GLuint>(indices.size());
        elements_handle = _create_elements_buffer(indices);
        positions_handle = _create_attribute_buffer(positions);
        _ownership.emplace();
    }

    void rendering_mesh_line::update(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
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
