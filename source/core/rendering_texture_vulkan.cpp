#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

        [[nodiscard]] bool _can_atlas_texture(const data_image& image)
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

        [[nodiscard]] bool _is_compressed_texture(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::s3tc_rgb4:
            case data_image_profile::s3tc_rgba8:
            case data_image_profile::etc2_rgb4:
            case data_image_profile::etc2_rgba8:
                return true;
            default:
                return false;
            }
        }

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

        [[nodiscard]] std::vector<uint8> _make_upload_pixels(const data_image& image)
        {
            if (_is_compressed_texture(image.profile)) {
                return image.pixels;
            }
            const uint32 _source_bytes_per_pixel = image.channels;
            const uint32 _bytes_per_pixel = _texture_bytes_per_pixel(image);
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _source_bytes_per_pixel;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel == _bytes_per_pixel) {
                return image.pixels;
            }
            if (image.profile != data_image_profile::rgba8888 || image.channels != 3) {
                LUCARIA_DEBUG_ERROR("Texture pixel data cannot be converted to its storage profile")
                return {};
            }
            std::vector<uint8> _pixels(static_cast<std::size_t>(image.width) * image.height * 4);
            for (uint32 _index = 0; _index < image.width * image.height; ++_index) {
                const std::size_t _source = static_cast<std::size_t>(_index) * 3;
                const std::size_t _destination = static_cast<std::size_t>(_index) * 4;
                std::copy_n(image.pixels.data() + _source, 3, _pixels.data() + _destination);
                _pixels[_destination + 3] = 255;
            }
            return _pixels;
        }

        void _destroy_dedicated_texture(rendering_texture& texture)
        {
            rendering_vulkan_remove_imgui_texture(texture.imgui_descriptor);
            rendering_vulkan_destroy_sampler(texture.sampler);
            rendering_vulkan_destroy_image(texture.image, texture.memory, texture.image_view);
            texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        void _create_empty_dedicated_texture(rendering_texture& texture, const uint32x2 size)
        {
            texture.profile = data_image_profile::rgba8888;
            texture.size = size;
            texture.uv_rect = { 0.f, 1.f, 1.f, -1.f };
            texture.format = rendering_vulkan_image_format(texture.profile);
            texture.is_dedicated_storage = true;
            rendering_vulkan_create_image(size, texture.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT, texture.image, texture.memory, texture.image_view, texture.layout);
            rendering_vulkan_create_sampler(texture.sampler);
        }

        void _upload_dedicated_texture(rendering_texture& texture, const data_image& image)
        {
            const std::vector<uint8> _pixels = _make_upload_pixels(image);
            if (_pixels.empty()) {
                LUCARIA_DEBUG_ERROR("Failed to prepare dedicated Vulkan texture")
                return;
            }
            _destroy_dedicated_texture(texture);
            texture.profile = image.profile;
            texture.size = { image.width, image.height };
            texture.format = rendering_vulkan_image_format(image.profile);
            texture.is_dedicated_storage = true;
            texture.allocation = {};
            texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
            rendering_vulkan_create_image(texture.size, texture.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, texture.image, texture.memory, texture.image_view, texture.layout);
            rendering_vulkan_create_sampler(texture.sampler);
            rendering_vulkan_upload_image(texture.image, texture.size, _pixels.data(), static_cast<VkDeviceSize>(_pixels.size()), VK_IMAGE_ASPECT_COLOR_BIT, texture.layout);
            texture.imgui_descriptor = rendering_vulkan_add_imgui_texture(texture.sampler, texture.image_view, texture.layout);
        }

    }

    rendering_texture::~rendering_texture()
    {
        _release();
    }

    void rendering_texture::_release() noexcept
    {
        if (!_ownership.owns()) {
            return;
        }
        if (!is_dedicated_storage && _registry != nullptr) {
            _registry->release(*this);
        } else if (is_dedicated_storage) {
            _destroy_dedicated_texture(*this);
        }
    }

    rendering_texture::rendering_texture(rendering_textures_registry& registry, const data_image& from)
        : profile(from.profile)
        , size(from.width, from.height)
    {
        if (_can_atlas_texture(from)) {
            is_dedicated_storage = false;
            _registry = &registry;
            _registry->upload(*this, from);
        } else {
            _upload_dedicated_texture(*this, from);
        }
        _ownership.emplace();
    }

    rendering_texture::rendering_texture(const uint32x2 size)
        : profile(data_image_profile::rgba8888)
        , size(size)
    {
        _create_empty_dedicated_texture(*this, size);
        is_dedicated_storage = true;
        _ownership.emplace();
    }

    void rendering_texture::resize(const uint32x2 new_size)
    {
        if (size == new_size) {
            return;
        }
        _release();
        _registry = nullptr;
        profile = data_image_profile::rgba8888;
        size = new_size;
        is_dedicated_storage = true;
        allocation = {};
        _create_empty_dedicated_texture(*this, new_size);
    }

    void rendering_texture::update(const data_image& from)
    {
        const bool _use_atlas = !is_dedicated_storage && _registry != nullptr && _can_atlas_texture(from);
        if (_use_atlas) {
            _release();
            profile = from.profile;
            size = { from.width, from.height };
            _registry->upload(*this, from);
            return;
        }
        if (!is_dedicated_storage) {
            _release();
            _registry = nullptr;
            image = VK_NULL_HANDLE;
            memory = VK_NULL_HANDLE;
            image_view = VK_NULL_HANDLE;
            sampler = VK_NULL_HANDLE;
            imgui_descriptor = VK_NULL_HANDLE;
        }
        _upload_dedicated_texture(*this, from);
    }

    ImTextureID rendering_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(imgui_descriptor);
    }

    ImVec2 rendering_texture::imgui_uv0() const
    {
        return { uv_rect.x, uv_rect.y };
    }

    ImVec2 rendering_texture::imgui_uv1() const
    {
        return { uv_rect.x + uv_rect.z, uv_rect.y + uv_rect.w };
    }

}
}

#endif
