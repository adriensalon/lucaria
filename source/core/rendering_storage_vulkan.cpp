#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/core/rendering_storage.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        constexpr uint32 _default_vertex_page_size = 64u * 1024u * 1024u;
        constexpr uint32 _default_element_page_size = 16u * 1024u * 1024u;
        constexpr uint32 _buffer_page_alignment = 256u;
        constexpr uint32 _default_texture_page_width = 4096;
        constexpr uint32 _default_texture_page_height = 4096;
        constexpr uint32 _texture_gutter = 1;

        [[nodiscard]] uint32 _texture_bytes_per_pixel(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return 2;
            default:
                return 4;
            }
        }

        [[nodiscard]] uint32 _texture_source_bytes_per_pixel(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return 2;
            default:
                return image.channels;
            }
        }

        [[nodiscard]] std::vector<uint8> _make_padded_pixels(const data_image& image)
        {
            const uint32 _bytes_per_pixel = _texture_bytes_per_pixel(image);
            const uint32 _source_bytes_per_pixel = _texture_source_bytes_per_pixel(image);
            if (image.width == 0 || image.height == 0 || _bytes_per_pixel == 0 || _source_bytes_per_pixel == 0) {
                return {};
            }
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _source_bytes_per_pixel;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel != _bytes_per_pixel && !(image.profile == data_image_profile::rgba8888 && image.channels == 3)) {
                LUCARIA_DEBUG_ERROR("Texture pixel data cannot be converted to its storage profile")
                return {};
            }
            const uint32 _padded_width = image.width + _texture_gutter * 2;
            const uint32 _padded_height = image.height + _texture_gutter * 2;
            std::vector<uint8> _padded(static_cast<std::size_t>(_padded_width) * _padded_height * _bytes_per_pixel);
            for (uint32 _y = 0; _y < _padded_height; ++_y) {
                const uint32 _source_y = std::min(std::max(_y, _texture_gutter) - _texture_gutter, image.height - 1);
                for (uint32 _x = 0; _x < _padded_width; ++_x) {
                    const uint32 _source_x = std::min(std::max(_x, _texture_gutter) - _texture_gutter, image.width - 1);
                    const std::size_t _source_offset = (static_cast<std::size_t>(_source_y) * image.width + _source_x) * _source_bytes_per_pixel;
                    const std::size_t _destination_offset = (static_cast<std::size_t>(_y) * _padded_width + _x) * _bytes_per_pixel;
                    if (_source_bytes_per_pixel == _bytes_per_pixel) {
                        std::copy_n(image.pixels.data() + _source_offset, _bytes_per_pixel, _padded.data() + _destination_offset);
                    } else {
                        std::copy_n(image.pixels.data() + _source_offset, _source_bytes_per_pixel, _padded.data() + _destination_offset);
                        _padded[_destination_offset + 3] = 255;
                    }
                }
            }
            return _padded;
        }

        [[nodiscard]] rendering_meshes_page _create_geometry_page(const uint32 required_vertex_size, const uint32 required_element_size)
        {
            rendering_meshes_page _page = {};
            _page.vertex_capacity = rendering_meshes_align_up(std::max(required_vertex_size, _default_vertex_page_size), _buffer_page_alignment);
            _page.element_capacity = rendering_meshes_align_up(std::max(required_element_size, _default_element_page_size), _buffer_page_alignment);
            rendering_vulkan_create_buffer(_page.vertex_capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _page.vertices_buffer, _page.vertices_memory);
            rendering_vulkan_create_buffer(_page.element_capacity, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _page.elements_buffer, _page.elements_memory);
            _page.free_vertices.free({ 0, _page.vertex_capacity });
            _page.free_elements.free({ 0, _page.element_capacity });
            return _page;
        }

        [[nodiscard]] rendering_textures_page _create_texture_page(const data_image_profile profile, const uint32x2 required_size)
        {
            rendering_textures_page _page = {};
            _page.profile = profile;
            _page.capacity = { std::max(required_size.x, _default_texture_page_width), std::max(required_size.y, _default_texture_page_height) };
            _page.format = rendering_vulkan_image_format(profile);
            rendering_vulkan_create_image(_page.capacity, _page.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, _page.image, _page.memory, _page.image_view, _page.layout);
            rendering_vulkan_create_sampler(_page.sampler);
            _page.free_pixels.push_back({ { 0, 0 }, _page.capacity });
            return _page;
        }

    }

    rendering_meshes_registry::~rendering_meshes_registry()
    {
        for (std::pair<const data_geometry_profile, rendering_meshes_buffer>& _pair : buffers) {
            rendering_meshes_buffer& _pool = _pair.second;
            for (rendering_meshes_page& _page : _pool.pages) {
                rendering_vulkan_destroy_buffer(_page.vertices_buffer, _page.vertices_memory);
                rendering_vulkan_destroy_buffer(_page.elements_buffer, _page.elements_memory);
            }
        }
    }

    void rendering_meshes_registry::upload(rendering_mesh& mesh, const std::vector<uint8>& vertices, const std::vector<uint32>& elements)
    {
        const uint32 _vertex_size = static_cast<uint32>(vertices.size());
        const uint32 _element_size = static_cast<uint32>(elements.size() * sizeof(uint32));
        rendering_meshes_buffer& _pool = assure_buffer(mesh.profile);
        std::optional<rendering_mesh_allocation> _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        if (!_allocation) {
            _pool.pages.push_back(_create_geometry_page(_vertex_size, _element_size));
            _allocation = _pool.allocate(_vertex_size, _element_size, _buffer_page_alignment, sizeof(uint32));
        }
        LUCARIA_DEBUG_ASSERT(_allocation, "Failed to allocate rendering mesh storage")
        if (!_allocation) {
            return;
        }
        rendering_meshes_page& _page = _pool.pages[_allocation->page];
        rendering_vulkan_upload_buffer(_page.vertices_buffer, _allocation->vertices.offset, vertices.data(), _allocation->vertices.size);
        rendering_vulkan_upload_buffer(_page.elements_buffer, _allocation->elements.offset, elements.data(), _allocation->elements.size);
        mesh.vertices_buffer = _page.vertices_buffer;
        mesh.elements_buffer = _page.elements_buffer;
        mesh.allocation = *_allocation;
    }

    void rendering_meshes_registry::release(rendering_mesh& mesh)
    {
        rendering_meshes_buffer* _pool = find_buffer(mesh.profile);
        if (_pool != nullptr) {
            _pool->free(mesh.allocation);
        }
        mesh.allocation = {};
        mesh.vertices_buffer = VK_NULL_HANDLE;
        mesh.elements_buffer = VK_NULL_HANDLE;
    }

    rendering_textures_registry::~rendering_textures_registry()
    {
        for (std::pair<const data_image_profile, rendering_textures_buffer>& _pair : buffers) {
            rendering_textures_buffer& _buffer = _pair.second;
            for (rendering_textures_page& _page : _buffer.pages) {
                rendering_vulkan_remove_imgui_texture(_page.descriptor);
                rendering_vulkan_destroy_sampler(_page.sampler);
                rendering_vulkan_destroy_image(_page.image, _page.memory, _page.image_view);
            }
        }
    }

    void rendering_textures_registry::upload(rendering_texture& texture, const data_image& image)
    {
        const uint32x2 _size = { image.width, image.height };
        const uint32x2 _padded_size = { _size.x + _texture_gutter * 2, _size.y + _texture_gutter * 2 };
        const std::vector<uint8> _padded_pixels = _make_padded_pixels(image);
        if (_padded_pixels.empty()) {
            LUCARIA_DEBUG_ERROR("Failed to prepare rendering texture storage")
            return;
        }
        rendering_textures_buffer& _buffer = assure_buffer(image.profile);
        std::optional<rendering_texture_allocation> _allocation = _buffer.allocate(_padded_size);
        if (!_allocation) {
            _buffer.pages.push_back(_create_texture_page(image.profile, _padded_size));
            _allocation = _buffer.allocate(_padded_size);
        }
        LUCARIA_DEBUG_ASSERT(_allocation, "Failed to allocate rendering texture storage")
        if (!_allocation) {
            return;
        }
        rendering_textures_page& _page = _buffer.pages[_allocation->page];
        rendering_vulkan_upload_image_region(_page.image, _allocation->pixels.offset, _padded_size, _padded_pixels.data(), static_cast<VkDeviceSize>(_padded_pixels.size()), VK_IMAGE_ASPECT_COLOR_BIT, _page.layout);
        if (_page.descriptor == VK_NULL_HANDLE) {
            _page.descriptor = rendering_vulkan_add_imgui_texture(_page.sampler, _page.image_view, _page.layout);
        }
        texture.profile = image.profile;
        texture.size = _size;
        texture.allocation = *_allocation;
        texture.image = _page.image;
        texture.image_view = _page.image_view;
        texture.sampler = _page.sampler;
        texture.imgui_descriptor = _page.descriptor;
        texture.format = _page.format;
        texture.layout = _page.layout;
        texture.uv_rect = {
            static_cast<float32>(_allocation->pixels.offset.x + _texture_gutter) / static_cast<float32>(_page.capacity.x),
            static_cast<float32>(_allocation->pixels.offset.y + _texture_gutter) / static_cast<float32>(_page.capacity.y),
            static_cast<float32>(_size.x) / static_cast<float32>(_page.capacity.x),
            static_cast<float32>(_size.y) / static_cast<float32>(_page.capacity.y)
        };
    }

    void rendering_textures_registry::release(rendering_texture& texture)
    {
        rendering_textures_buffer* _buffer = find_buffer(texture.profile);
        if (_buffer != nullptr) {
            _buffer->free(texture.allocation);
        }
        texture.allocation = {};
        texture.image = VK_NULL_HANDLE;
        texture.memory = VK_NULL_HANDLE;
        texture.image_view = VK_NULL_HANDLE;
        texture.sampler = VK_NULL_HANDLE;
        texture.imgui_descriptor = VK_NULL_HANDLE;
        texture.size = {};
        texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
    }

}
}

#endif
