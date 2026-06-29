#include <algorithm>
#include <cstring>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>

namespace lucaria {
namespace detail {
    namespace {

        struct _packed_vertex_layout {
            uint32 stride = 0;
            int format = GU_VERTEX_32BITF;
        };

        [[nodiscard]] static uint32 _get_vertices_count(const data_geometry& from)
        {
            if (from.vertices_count != 0) {
                return from.vertices_count;
            }
            if (!from.positions.empty()) {
                return static_cast<uint32>(from.positions.size());
            }
            if (!from.texcoords.empty()) {
                return static_cast<uint32>(from.texcoords.size());
            }
            if (!from.normals.empty()) {
                return static_cast<uint32>(from.normals.size());
            }
            if (!from.colors.empty()) {
                return static_cast<uint32>(from.colors.size());
            }
            return 0;
        }

        [[nodiscard]] static uint32 _pack_color(const float32x4 color)
        {
            const auto _byte = [](float32 value) {
                return static_cast<uint32>(std::clamp(value, 0.f, 1.f) * 255.f);
            };
            return _byte(color.x) | (_byte(color.y) << 8) | (_byte(color.z) << 16) | (_byte(color.w) << 24);
        }

        template <typename ValueType>
        static void _append_value(std::vector<uint8>& bytes, const ValueType& value)
        {
            const std::size_t _offset = bytes.size();
            bytes.resize(_offset + sizeof(ValueType));
            std::memcpy(bytes.data() + _offset, &value, sizeof(ValueType));
        }

        [[nodiscard]] static _packed_vertex_layout _make_layout(const data_geometry& from)
        {
            _packed_vertex_layout _layout = {};
            if (!from.texcoords.empty()) {
                _layout.format |= GU_TEXTURE_32BITF;
                _layout.stride += sizeof(float32x2);
            }
            if (!from.colors.empty()) {
                _layout.format |= GU_COLOR_8888;
                _layout.stride += sizeof(uint32);
            }
            if (!from.normals.empty()) {
                _layout.format |= GU_NORMAL_32BITF;
                _layout.stride += sizeof(float32x3);
            }
            _layout.stride += sizeof(float32x3);
            return _layout;
        }

        [[nodiscard]] static std::vector<uint8> _pack_vertices(const data_geometry& from, const uint32 vertices_count)
        {
            std::vector<uint8> _vertices = {};
            _vertices.reserve(static_cast<std::size_t>(vertices_count) * _make_layout(from).stride);
            for (uint32 _index = 0; _index < vertices_count; ++_index) {
                if (!from.texcoords.empty()) {
                    LUCARIA_DEBUG_ASSERT(_index < from.texcoords.size(), "Invalid mesh texcoord attribute size")
                    _append_value(_vertices, from.texcoords[_index]);
                }
                if (!from.colors.empty()) {
                    LUCARIA_DEBUG_ASSERT(_index < from.colors.size(), "Invalid mesh color attribute size")
                    const uint32 _color = _pack_color(from.colors[_index]);
                    _append_value(_vertices, _color);
                }
                if (!from.normals.empty()) {
                    LUCARIA_DEBUG_ASSERT(_index < from.normals.size(), "Invalid mesh normal attribute size")
                    _append_value(_vertices, from.normals[_index]);
                }
                LUCARIA_DEBUG_ASSERT(_index < from.positions.size(), "PSPGU meshes require positions")
                _append_value(_vertices, from.positions[_index]);
            }
            return _vertices;
        }

        [[nodiscard]] static std::vector<uint16> _pack_indices(const std::vector<uint32x3>& indices)
        {
            std::vector<uint16> _packed = {};
            _packed.reserve(indices.size() * 3);
            for (const uint32x3& _index : indices) {
                LUCARIA_DEBUG_ASSERT(_index.x <= 0xffff && _index.y <= 0xffff && _index.z <= 0xffff, "PSPGU only supports 16-bit mesh indices")
                _packed.push_back(static_cast<uint16>(_index.x));
                _packed.push_back(static_cast<uint16>(_index.y));
                _packed.push_back(static_cast<uint16>(_index.z));
            }
            return _packed;
        }

        [[nodiscard]] static std::vector<uint16> _pack_line_indices(const std::vector<uint32x2>& indices)
        {
            std::vector<uint16> _packed = {};
            _packed.reserve(indices.size() * 2);
            for (const uint32x2& _index : indices) {
                LUCARIA_DEBUG_ASSERT(_index.x <= 0xffff && _index.y <= 0xffff, "PSPGU only supports 16-bit mesh indices")
                _packed.push_back(static_cast<uint16>(_index.x));
                _packed.push_back(static_cast<uint16>(_index.y));
            }
            return _packed;
        }

    }

    rendering_meshes_registry::~rendering_meshes_registry() = default;

    void rendering_meshes_registry::upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements)
    {
        mesh.vertex_bytes = vertices;
        mesh.elements_16.clear();
        mesh.elements_16.reserve(elements.size());
        for (const uint32 _element : elements) {
            LUCARIA_DEBUG_ASSERT(_element <= 0xffff, "PSPGU only supports 16-bit mesh indices")
            mesh.elements_16.push_back(static_cast<uint16>(_element));
        }
        sceKernelDcacheWritebackInvalidateAll();
    }

    void rendering_meshes_registry::release(rendering_mesh& mesh)
    {
        mesh.vertex_bytes.clear();
        mesh.elements_16.clear();
        mesh.allocation = {};
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
        const uint32 _vertices_count = _get_vertices_count(from);
        const _packed_vertex_layout _layout = _make_layout(from);
        profile = from.profile;
        vertex_stride = _layout.stride;
        vertex_format = _layout.format;
        vertex_bytes = _pack_vertices(from, _vertices_count);
        elements_16 = _pack_indices(from.indices);
        invposes = from.invposes;
        size = static_cast<uint32>(elements_16.size());
        _ownership.emplace();
        sceKernelDcacheWritebackInvalidateAll();
    }

    rendering_mesh_line::~rendering_mesh_line() = default;

    rendering_mesh_line::rendering_mesh_line(const data_geometry& from)
        : positions(from.positions)
    {
        std::vector<uint32x2> _indices = {};
        _indices.reserve(from.indices.size() * 3);
        for (const uint32x3& _triangle : from.indices) {
            _indices.emplace_back(_triangle.x, _triangle.y);
            _indices.emplace_back(_triangle.y, _triangle.z);
            _indices.emplace_back(_triangle.z, _triangle.x);
        }
        elements_16 = _pack_line_indices(_indices);
        size = static_cast<uint32>(elements_16.size());
        _ownership.emplace();
        sceKernelDcacheWritebackInvalidateAll();
    }

    rendering_mesh_line::rendering_mesh_line(const std::vector<float32x3>& from_positions, const std::vector<uint32x2>& indices)
        : size(static_cast<uint32>(indices.size() * 2))
        , positions(from_positions)
        , elements_16(_pack_line_indices(indices))
    {
        _ownership.emplace();
        sceKernelDcacheWritebackInvalidateAll();
    }

    void rendering_mesh_line::update(const std::vector<float32x3>& from_positions, const std::vector<uint32x2>& indices)
    {
        positions = from_positions;
        elements_16 = _pack_line_indices(indices);
        size = static_cast<uint32>(elements_16.size());
        sceKernelDcacheWritebackInvalidateAll();
    }

}
}
