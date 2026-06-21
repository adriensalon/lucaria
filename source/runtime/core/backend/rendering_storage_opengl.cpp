#include <algorithm>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/core/rendering_storage.hpp>

namespace lucaria {
namespace detail {

    namespace {

        constexpr uint32 _default_vertex_page_size = 64u * 1024u * 1024u;
        constexpr uint32 _default_element_page_size = 16u * 1024u * 1024u;
        constexpr uint32 _buffer_page_alignment = 256u;

        [[nodiscard]] static rendering_meshes_page _create_geometry_page(const uint32 required_vertex_size, const uint32 required_element_size)
        {
            rendering_meshes_page _page = {};
            _page.vertex_capacity = rendering_align_up(std::max(required_vertex_size, _default_vertex_page_size), _buffer_page_alignment);
            _page.element_capacity = rendering_align_up(std::max(required_element_size, _default_element_page_size), _buffer_page_alignment);
            glGenBuffers(1, &_page.vertices_id);
            glBindBuffer(GL_ARRAY_BUFFER, _page.vertices_id);
            glBufferData(GL_ARRAY_BUFFER, _page.vertex_capacity, nullptr, GL_STATIC_DRAW);
            glGenBuffers(1, &_page.elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _page.elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, _page.element_capacity, nullptr, GL_STATIC_DRAW);
            _page.vertices.free({ 0, _page.vertex_capacity });
            _page.elements.free({ 0, _page.element_capacity });
            return _page;
        }

    }

    rendering_meshes_registry::~rendering_meshes_registry()
    {
        for (std::pair<const data_geometry_profile, rendering_meshes_buffer>& _pair : pools) {
            rendering_meshes_buffer& _pool = _pair.second;
            for (rendering_meshes_page& _page : _pool.pages) {
                if (_page.vertices_id != 0) {
                    glDeleteBuffers(1, &_page.vertices_id);
                    _page.vertices_id = 0;
                }
                if (_page.elements_id != 0) {
                    glDeleteBuffers(1, &_page.elements_id);
                    _page.elements_id = 0;
                }
            }
        }
    }

    void rendering_meshes_registry::upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements)
    {
        const uint32 _vertex_size = static_cast<uint32>(vertices.size());
        const uint32 _element_size = static_cast<uint32>(elements.size() * sizeof(uint32));
        rendering_meshes_buffer& _pool = assure_pool(mesh.profile);
        std::optional<rendering_mesh_allocation> _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        if (!_allocation) {
            _pool.pages.push_back(_create_geometry_page(_vertex_size, _element_size));
            _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        }
        LUCARIA_DEBUG_ASSERT(_allocation, "Failed to allocate rendering mesh storage")
        rendering_meshes_page& _page = _pool.pages[_allocation->page];
        if (_allocation->vertices.size != 0) {
            glBindBuffer(GL_ARRAY_BUFFER, _page.vertices_id);
            glBufferSubData(GL_ARRAY_BUFFER, _allocation->vertices.offset, _allocation->vertices.size, vertices.data());
        }
        if (_allocation->elements.size != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _page.elements_id);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, _allocation->elements.offset, _allocation->elements.size, elements.data());
        }
        mesh.vertices_id = _page.vertices_id;
        mesh.elements_id = _page.elements_id;
        mesh.allocation = _allocation.value();
    }

    void rendering_meshes_registry::release(rendering_mesh& mesh)
    {
        rendering_meshes_buffer* _pool = find_pool(mesh.profile);
        if (_pool == nullptr) {
            return;
        }
        _pool->free(mesh.allocation);
        mesh.allocation = {};
        mesh.vertices_id = 0;
        mesh.elements_id = 0;
    }

}
}
