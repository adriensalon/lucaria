#include <algorithm>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_mesh.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_storage.hpp>

namespace lucaria {
namespace detail {

    namespace {

        constexpr uint32 _default_vertex_page_size = 64u * 1024u * 1024u;
        constexpr uint32 _default_element_page_size = 16u * 1024u * 1024u;
        constexpr uint32 _buffer_page_alignment = 256u;

        constexpr uint32 _default_texture_page_width = 4096;
        constexpr uint32 _default_texture_page_height = 4096;
        constexpr uint32 _texture_gutter = 1;
        constexpr GLenum _rgb565_internal_format = 0x8D62;

        [[nodiscard]] static GLenum _texture_internal_format(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::rgba8888:
                return GL_RGBA8;
            case data_image_profile::rgb565:
                return _rgb565_internal_format;
            case data_image_profile::rgba5551:
                return GL_RGB5_A1;
            case data_image_profile::rgba4444:
                return GL_RGBA4;
            default:
                return GL_RGBA8;
            }
        }

        [[nodiscard]] static GLenum _texture_format(const data_image_profile profile, const uint32 channels)
        {
            if (profile == data_image_profile::rgb565) {
                return GL_RGB;
            }
            if (channels == 3) {
                return GL_RGB;
            }
            return GL_RGBA;
        }

        [[nodiscard]] static GLenum _texture_type(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::rgb565:
                return GL_UNSIGNED_SHORT_5_6_5;
            case data_image_profile::rgba5551:
                return GL_UNSIGNED_SHORT_5_5_5_1;
            case data_image_profile::rgba4444:
                return GL_UNSIGNED_SHORT_4_4_4_4;
            default:
                return GL_UNSIGNED_BYTE;
            }
        }

        [[nodiscard]] static uint32 _texture_bytes_per_pixel(const data_image& image)
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

        [[nodiscard]] static std::vector<uint8> _make_padded_pixels(const data_image& image)
        {
            const uint32 _bytes_per_pixel = _texture_bytes_per_pixel(image);
            if (image.width == 0 || image.height == 0 || _bytes_per_pixel == 0) {
                return {};
            }
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _bytes_per_pixel;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return {};
            }
            const uint32 _padded_width = image.width + _texture_gutter * 2;
            const uint32 _padded_height = image.height + _texture_gutter * 2;
            std::vector<uint8> _padded(static_cast<std::size_t>(_padded_width) * _padded_height * _bytes_per_pixel);
            for (uint32 _y = 0; _y < _padded_height; ++_y) {
                const uint32 _source_y = std::min(std::max(_y, _texture_gutter) - _texture_gutter, image.height - 1);
                for (uint32 _x = 0; _x < _padded_width; ++_x) {
                    const uint32 _source_x = std::min(std::max(_x, _texture_gutter) - _texture_gutter, image.width - 1);
                    const std::size_t _source_offset = (static_cast<std::size_t>(_source_y) * image.width + _source_x) * _bytes_per_pixel;
                    const std::size_t _destination_offset = (static_cast<std::size_t>(_y) * _padded_width + _x) * _bytes_per_pixel;
                    std::copy_n(image.pixels.data() + _source_offset, _bytes_per_pixel, _padded.data() + _destination_offset);
                }
            }
            return _padded;
        }

        [[nodiscard]] static bool _can_atlas_texture(const data_image& image)
        {
            switch (image.profile) {
            case data_image_profile::rgba8888:
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return true;
            default:
                return false;
            }
        }

        [[nodiscard]] static rendering_textures_page _create_texture_page(const data_image_profile profile, const uint32x2 required_size)
        {
            rendering_textures_page _page = {};
            _page.profile = profile;
            _page.capacity = { std::max(required_size.x, _default_texture_page_width), std::max(required_size.y, _default_texture_page_height) };
            glGenTextures(1, &_page.texture_id);
            glBindTexture(GL_TEXTURE_2D, _page.texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            const GLenum _format = profile == data_image_profile::rgb565 ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D, 0, _texture_internal_format(profile), _page.capacity.x, _page.capacity.y, 0, _format, _texture_type(profile), nullptr);
            _page.free_pixels.push_back({ { 0, 0 }, _page.capacity });
            return _page;
        }

        [[nodiscard]] static rendering_meshes_page _create_geometry_page(const uint32 required_vertex_size, const uint32 required_element_size)
        {
            rendering_meshes_page _page = {};
            _page.vertex_capacity = rendering_meshes_align_up(std::max(required_vertex_size, _default_vertex_page_size), _buffer_page_alignment);
            _page.element_capacity = rendering_meshes_align_up(std::max(required_element_size, _default_element_page_size), _buffer_page_alignment);
            glGenBuffers(1, &_page.vertices_id);
            glBindBuffer(GL_ARRAY_BUFFER, _page.vertices_id);
            glBufferData(GL_ARRAY_BUFFER, _page.vertex_capacity, nullptr, GL_STATIC_DRAW);
            glGenBuffers(1, &_page.elements_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _page.elements_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, _page.element_capacity, nullptr, GL_STATIC_DRAW);
            _page.free_vertices.free({ 0, _page.vertex_capacity });
            _page.free_elements.free({ 0, _page.element_capacity });
            return _page;
        }

    }

    rendering_meshes_registry::~rendering_meshes_registry()
    {
        for (std::pair<const data_geometry_profile, rendering_meshes_buffer>& _pair : buffers) {
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
        rendering_meshes_buffer& _pool = assure_buffer(mesh.profile);
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
        rendering_meshes_buffer* _pool = find_buffer(mesh.profile);
        if (_pool == nullptr) {
            return;
        }
        _pool->free(mesh.allocation);
        mesh.allocation = {};
        mesh.vertices_id = 0;
        mesh.elements_id = 0;
    }

    rendering_textures_registry::~rendering_textures_registry()
    {
        for (std::pair<const data_image_profile, rendering_textures_buffer>& _pair : buffers) {
            rendering_textures_buffer& _buffer = _pair.second;
            for (rendering_textures_page& _page : _buffer.pages) {
                if (_page.texture_id != 0) {
                    glDeleteTextures(1, &_page.texture_id);
                    _page.texture_id = 0;
                }
            }
        }
    }

    void rendering_textures_registry::upload(rendering_texture& texture, const data_image& image)
    {
        LUCARIA_DEBUG_ASSERT(_can_atlas_texture(image), "Texture profile cannot be atlased yet")
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
        glBindTexture(GL_TEXTURE_2D, _page.texture_id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, _allocation->pixels.offset.x, _allocation->pixels.offset.y, _padded_size.x, _padded_size.y,
            _texture_format(image.profile, image.channels),
            _texture_type(image.profile),
            _padded_pixels.data());
        texture.profile = image.profile;
        texture.size = _size;
        texture.allocation = *_allocation;
        texture.texture_id = _page.texture_id;
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
        if (_buffer == nullptr) {
            return;
        }
        _buffer->free(texture.allocation);
        texture.allocation = {};
        texture.texture_id = 0;
        texture.size = {};
        texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
    }

}
}
