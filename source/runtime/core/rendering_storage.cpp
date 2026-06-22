#include <lucaria/core/rendering_storage.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace lucaria {
namespace detail {

	// meshes

    std::optional<rendering_mesh_range> rendering_mesh_free_list::allocate(const uint32 size, const uint32 alignment)
    {
        if (size == 0) {
            return rendering_mesh_range {};
        }
        for (uint32 _index = 0; _index < ranges.size(); ++_index) {
            rendering_mesh_range& _range = ranges[_index];
            const uint32 _aligned_offset = rendering_meshes_align_up(_range.offset, alignment);
            const uint32 _padding = _aligned_offset - _range.offset;
            if (_range.size < _padding + size) {
                continue;
            }
            const uint32 _old_end = _range.offset + _range.size;
            const uint32 _new_end = _aligned_offset + size;
            rendering_mesh_range _result {
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
                rendering_mesh_range _tail {
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

    void rendering_mesh_free_list::free(const rendering_mesh_range range)
    {
        if (range.size == 0) {
            return;
        }
        ranges.push_back(range);
        std::sort(ranges.begin(), ranges.end(), [](const rendering_mesh_range& a, const rendering_mesh_range& b) {
            return a.offset < b.offset;
        });
        std::vector<rendering_mesh_range> _merged;
        _merged.reserve(ranges.size());
        for (const rendering_mesh_range& _current : ranges) {
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
            std::optional<rendering_mesh_range> _vertices_range = _page.free_vertices.allocate(vertex_size, vertex_alignment);
            if (!_vertices_range) {
                continue;
            }
            std::optional<rendering_mesh_range> _elements_range = _page.free_elements.allocate(element_size, element_alignment);
            if (!_elements_range) {
                _page.free_vertices.free(*_vertices_range);
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
        _page.free_vertices.free(allocation.vertices);
        _page.free_elements.free(allocation.elements);
        if (_page.allocations > 0) {
            --_page.allocations;
        }
    }

	// textures

    std::optional<rendering_texture_allocation> rendering_textures_page::allocate(const uint32x2 size)
    {
        if (size.x == 0 || size.y == 0) {
            return std::nullopt;
        }
        uint32 _best_index = rendering_invalid_page;
        std::uint64_t _best_waste = std::numeric_limits<std::uint64_t>::max();
        for (uint32 _index = 0; _index < free_pixels.size(); ++_index) {
            const rendering_texture_range& _free = free_pixels[_index];
            if (_free.size.x >= size.x && _free.size.y >= size.y) {
                const std::uint64_t _free_area = static_cast<std::uint64_t>(_free.size.x) * _free.size.y;
                const std::uint64_t _required_area = static_cast<std::uint64_t>(size.x) * size.y;
                const std::uint64_t _waste = _free_area - _required_area;
                if (_waste < _best_waste) {
                    _best_index = _index;
                    _best_waste = _waste;
                }
            }
        }
        if (_best_index == rendering_invalid_page) {
            return std::nullopt;
        }

        const rendering_texture_range _free = free_pixels[_best_index];
        free_pixels.erase(free_pixels.begin() + _best_index);
        rendering_texture_allocation _allocation = {};
        _allocation.pixels.offset = _free.offset;
        _allocation.pixels.size = size;
        const uint32 _right_width = _free.size.x - size.x;
        const uint32 _bottom_height = _free.size.y - size.y;
        if (_right_width != 0) {
            free_pixels.push_back({ { _free.offset.x + size.x, _free.offset.y },
                { _right_width, size.y } });
        }
        if (_bottom_height != 0) {
            free_pixels.push_back({ { _free.offset.x, _free.offset.y + size.y },
                { _free.size.x, _bottom_height } });
        }
        ++allocations;
        return _allocation;
    }

    void rendering_textures_page::free(const rendering_texture_allocation allocation)
    {
        if (allocation.pixels.size.x == 0 || allocation.pixels.size.y == 0) {
            return;
        }
        free_pixels.push_back(allocation.pixels);
        bool _did_merge = true;
        while (_did_merge) {
            _did_merge = false;
            for (uint32 _first = 0; _first < free_pixels.size() && !_did_merge; ++_first) {
                for (uint32 _second = _first + 1; _second < free_pixels.size(); ++_second) {
                    rendering_texture_range& _a = free_pixels[_first];
                    const rendering_texture_range& _b = free_pixels[_second];
                    if (_a.offset.y == _b.offset.y && _a.size.y == _b.size.y
                        && (_a.offset.x + _a.size.x == _b.offset.x || _b.offset.x + _b.size.x == _a.offset.x)) {
                        _a.offset.x = std::min(_a.offset.x, _b.offset.x);
                        _a.size.x += _b.size.x;
                    } else if (_a.offset.x == _b.offset.x && _a.size.x == _b.size.x
                        && (_a.offset.y + _a.size.y == _b.offset.y || _b.offset.y + _b.size.y == _a.offset.y)) {
                        _a.offset.y = std::min(_a.offset.y, _b.offset.y);
                        _a.size.y += _b.size.y;
                    } else {
                        continue;
                    }
                    free_pixels.erase(free_pixels.begin() + _second);
                    _did_merge = true;
                    break;
                }
            }
        }
        if (allocations > 0) {
            --allocations;
        }
    }

    std::optional<rendering_texture_allocation> rendering_textures_buffer::allocate(const uint32x2 size)
    {
        for (uint32 _page_index = 0; _page_index < pages.size(); ++_page_index) {
            std::optional<rendering_texture_allocation> _allocation = pages[_page_index].allocate(size);
            if (!_allocation) {
                continue;
            }
            _allocation->page = _page_index;
            return _allocation;
        }
        return std::nullopt;
    }

    void rendering_textures_buffer::free(const rendering_texture_allocation allocation)
    {
        if (allocation.page >= pages.size()) {
            return;
        }
        pages[allocation.page].free(allocation);
    }

	//

    rendering_meshes_buffer& rendering_meshes_registry::assure_buffer(const data_geometry_profile profile)
    {
        rendering_meshes_buffer& _buffer = buffers[profile];
        _buffer.profile = profile;
        return _buffer;
    }

    rendering_meshes_buffer* rendering_meshes_registry::find_buffer(const data_geometry_profile profile)
    {
        auto _iterator = buffers.find(profile);
        if (_iterator == buffers.end()) {
            return nullptr;
        }
        return &_iterator->second;
    }

    rendering_textures_buffer& rendering_textures_registry::assure_buffer(const data_image_profile profile)
    {
        rendering_textures_buffer& _buffer = buffers[profile];
        _buffer.profile = profile;
        return _buffer;
    }

    rendering_textures_buffer* rendering_textures_registry::find_buffer(const data_image_profile profile)
    {
        auto _iterator = buffers.find(profile);
        if (_iterator == buffers.end()) {
            return nullptr;
        }
        return &_iterator->second;
    }

    uint32 rendering_meshes_align_up(const uint32 value, const uint32 alignment)
    {
        if (alignment == 0) {
            return value;
        }
        return (value + alignment - 1) & ~(alignment - 1);
    }

}
}
