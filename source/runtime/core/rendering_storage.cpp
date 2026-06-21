#include <lucaria/core/rendering_storage.hpp>

namespace lucaria {
namespace detail {

    std::optional<rendering_allocator_range> rendering_allocator_free_list::allocate(const uint32 size, const uint32 alignment)
    {
        if (size == 0) {
            return rendering_allocator_range {};
        }
        for (uint32 _index = 0; _index < ranges.size(); ++_index) {
            rendering_allocator_range& _range = ranges[_index];
            const uint32 _aligned_offset = rendering_align_up(_range.offset, alignment);
            const uint32 _padding = _aligned_offset - _range.offset;
            if (_range.size < _padding + size) {
                continue;
            }
            const uint32 _old_end = _range.offset + _range.size;
            const uint32 _new_end = _aligned_offset + size;
            rendering_allocator_range _result {
                _aligned_offset,
                size
            };
            if (_padding == 0 && _new_end == _old_end) {
                ranges.erase(ranges.begin() + _index);
            } else if (_padding == 0) {
                _range.offset = _new_end;
                _range.size = _old_end - _new_end;
            } else if (_new_end == _old_end) {
                _range.size = _padding;
            } else {
                rendering_allocator_range _tail {
                    _new_end,
                    _old_end - _new_end
                };
                _range.size = _padding;
                ranges.insert(ranges.begin() + _index + 1, _tail);
            }
            return _result;
        }
        return std::nullopt;
    }

    void rendering_allocator_free_list::free(const rendering_allocator_range range)
    {
        if (range.size == 0) {
            return;
        }
        ranges.push_back(range);
        std::sort(ranges.begin(), ranges.end(), [](const rendering_allocator_range& a, const rendering_allocator_range& b) {
            return a.offset < b.offset;
        });
        std::vector<rendering_allocator_range> _merged;
        _merged.reserve(ranges.size());
        for (const rendering_allocator_range& _current : ranges) {
            if (!_merged.empty() && _merged.back().offset + _merged.back().size == _current.offset) {
                _merged.back().size += _current.size;
            } else {
                _merged.push_back(_current);
            }
        }
        ranges = std::move(_merged);
    }

    std::optional<rendering_mesh_allocation> rendering_meshes_buffer::allocate(const uint32 vertex_size, const uint32 element_size, const uint32 vertex_alignment, const uint32 element_alignment)
    {
        for (uint32 _page_index = 0; _page_index < pages.size(); ++_page_index) {
            rendering_meshes_page& _page = pages[_page_index];
            std::optional<rendering_allocator_range> _vertices_range = _page.vertices.allocate(vertex_size, vertex_alignment);
            if (!_vertices_range) {
                continue;
            }
            std::optional<rendering_allocator_range> _elements_range = _page.elements.allocate(element_size, element_alignment);
            if (!_elements_range) {
                _page.vertices.free(*_vertices_range);
                continue;
            }
            ++_page.allocations;
            return rendering_mesh_allocation {
                _page_index,
                *_vertices_range,
                *_elements_range
            };
        }
        return std::nullopt;
    }

    void rendering_meshes_buffer::free(const rendering_mesh_allocation allocation)
    {
        if (allocation.page >= pages.size()) {
            return;
        }
        rendering_meshes_page& _page = pages[allocation.page];
        _page.vertices.free(allocation.vertices);
        _page.elements.free(allocation.elements);
        if (_page.allocations > 0) {
            --_page.allocations;
        }
    }

    rendering_meshes_buffer& rendering_meshes_registry::assure_pool(const data_geometry_profile profile)
    {
        rendering_meshes_buffer& _pool = pools[profile];
        _pool.profile = profile;
        return _pool;
    }

    rendering_meshes_buffer* rendering_meshes_registry::find_pool(const data_geometry_profile profile)
    {
        auto _iterator = pools.find(profile);
        if (_iterator == pools.end()) {
            return nullptr;
        }
        return &_iterator->second;
    }

    uint32 rendering_align_up(const uint32 value, const uint32 alignment)
    {
        if (alignment == 0) {
            return value;
        }
        return (value + alignment - 1) & ~(alignment - 1);
    }

}
}
