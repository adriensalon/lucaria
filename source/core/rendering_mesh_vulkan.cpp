#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>
#include <cstring>
#include <set>

#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        struct _packed_vertex_layout {
            uint32 stride = 0;
            std::unordered_map<data_vertex_attribute, rendering_mesh_range> attributes = {};
        };

        [[nodiscard]] uint32 _get_vertices_count(const data_geometry& from)
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

        [[nodiscard]] uint32 _attribute_storage_size(const data_vertex_attribute attribute)
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

        [[nodiscard]] _packed_vertex_layout _make_packed_vertex_layout(const data_geometry& from)
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
        void _write_packed_attribute(std::vector<uint8>& packed, const _packed_vertex_layout& layout, const data_vertex_attribute attribute, const std::vector<VectorType>& values, const uint32 vertices_count)
        {
            if (values.empty()) {
                return;
            }
            const auto _iterator = layout.attributes.find(attribute);
            if (_iterator == layout.attributes.end()) {
                return;
            }
            const rendering_mesh_range& _attribute = _iterator->second;
            for (uint32 _index = 0; _index < vertices_count; ++_index) {
                uint8* _destination = packed.data() + static_cast<std::size_t>(_index) * layout.stride + _attribute.offset;
                std::memcpy(_destination, &values[_index], sizeof(VectorType));
            }
        }

        [[nodiscard]] std::vector<uint8> _pack_vertices(const data_geometry& from, const _packed_vertex_layout& layout)
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

        [[nodiscard]] std::vector<uint32> _pack_triangle_indices(const std::vector<uint32x3>& indices)
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

        [[nodiscard]] std::vector<uint32x2> _generate_line_indices(const std::vector<uint32x3>& triangle_indices)
        {
            std::set<std::pair<uint32, uint32>> _edges;
            for (const uint32x3& _triangle : triangle_indices) {
                _edges.insert(std::minmax(_triangle.x, _triangle.y));
                _edges.insert(std::minmax(_triangle.y, _triangle.z));
                _edges.insert(std::minmax(_triangle.z, _triangle.x));
            }
            std::vector<uint32x2> _line_indices;
            for (const std::pair<uint32, uint32>& _edge : _edges) {
                _line_indices.emplace_back(_edge.first, _edge.second);
            }
            return _line_indices;
        }

        [[nodiscard]] std::vector<uint32> _pack_line_indices(const std::vector<uint32x2>& indices)
        {
            std::vector<uint32> _packed;
            _packed.reserve(indices.size() * 2);
            for (const uint32x2& _index : indices) {
                _packed.push_back(_index.x);
                _packed.push_back(_index.y);
            }
            return _packed;
        }

        void _destroy_line_mesh(rendering_mesh_line& mesh)
        {
            rendering_vulkan_destroy_buffer(mesh.positions_buffer, mesh.positions_memory);
            rendering_vulkan_destroy_buffer(mesh.elements_buffer, mesh.elements_memory);
        }

        void _upload_line_mesh(rendering_mesh_line& mesh, const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
        {
            const std::vector<uint32> _elements = _pack_line_indices(indices);
            mesh.size = static_cast<uint32>(_elements.size());

            const VkDeviceSize _positions_size = static_cast<VkDeviceSize>(positions.size() * sizeof(float32x3));
            const VkDeviceSize _elements_size = static_cast<VkDeviceSize>(_elements.size() * sizeof(uint32));
            if (_positions_size == 0 || _elements_size == 0) {
                return;
            }
            rendering_vulkan_create_buffer(_positions_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.positions_buffer, mesh.positions_memory);
            rendering_vulkan_create_buffer(_elements_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.elements_buffer, mesh.elements_memory);
            rendering_vulkan_upload_buffer(mesh.positions_buffer, 0, positions.data(), _positions_size);
            rendering_vulkan_upload_buffer(mesh.elements_buffer, 0, _elements.data(), _elements_size);
        }

    }

    rendering_mesh::~rendering_mesh()
    {
        if (_ownership.owns() && _registry != nullptr) {
            _registry->release(*this);
        }
    }

    rendering_mesh::rendering_mesh(rendering_meshes_registry& registry, const data_geometry& from)
        : _registry(&registry)
    {
        const _packed_vertex_layout _layout = _make_packed_vertex_layout(from);
        const std::vector<uint8> _packed_vertices = _pack_vertices(from, _layout);
        const std::vector<uint32> _packed_indices = _pack_triangle_indices(from.indices);
        profile = from.profile;
        vertex_stride = _layout.stride;
        for (const std::pair<const data_vertex_attribute, rendering_mesh_range>& _pair : _layout.attributes) {
            attribute_offsets[_pair.first] = _pair.second.offset;
        }
        _registry->upload(*this, _packed_vertices, _packed_indices);
        invposes = from.invposes;
        size = static_cast<uint32>(_packed_indices.size());
        _ownership.emplace();
    }

    rendering_mesh_line::~rendering_mesh_line()
    {
        if (_ownership.owns()) {
            _destroy_line_mesh(*this);
        }
    }

    rendering_mesh_line::rendering_mesh_line(const data_geometry& from)
    {
        _upload_line_mesh(*this, from.positions, _generate_line_indices(from.indices));
        _ownership.emplace();
    }

    rendering_mesh_line::rendering_mesh_line(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
    {
        _upload_line_mesh(*this, positions, indices);
        _ownership.emplace();
    }

    void rendering_mesh_line::update(const std::vector<float32x3>& positions, const std::vector<uint32x2>& indices)
    {
        _destroy_line_mesh(*this);
        _upload_line_mesh(*this, positions, indices);
    }

}
}

#endif
