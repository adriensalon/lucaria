#if defined(LUCARIA_BACKEND_VULKAN)

#include <algorithm>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_cubemap.hpp>
#include <lucaria/core/rendering_vulkan.hpp>

namespace lucaria {
namespace detail {
    namespace {

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
                LUCARIA_DEBUG_ERROR("Cubemap pixel data is smaller than its dimensions and profile require")
                return {};
            }
            if (_source_bytes_per_pixel == _bytes_per_pixel) {
                return image.pixels;
            }
            if (image.profile != data_image_profile::rgba8888 || image.channels != 3) {
                LUCARIA_DEBUG_ERROR("Cubemap pixel data cannot be converted to its storage profile")
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

        void _upload_cubemap(rendering_cubemap& cubemap, const std::array<data_image, 6>& images)
        {
            constexpr std::array<uint32, 6> _vulkan_layer_to_lucaria_face = {
                0, // +X
                3, // -X
                1, // +Y
                4, // -Y
                2, // +Z
                5  // -Z
            };
            cubemap.profile = images[0].profile;
            cubemap.size = { images[0].width, images[0].height };
            cubemap.format = rendering_vulkan_image_format(cubemap.profile);
            rendering_vulkan_create_image(cubemap.size, cubemap.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, cubemap.image, cubemap.memory, cubemap.image_view, cubemap.layout, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
            rendering_vulkan_create_sampler(cubemap.sampler);
            for (uint32 _layer = 0; _layer < 6; ++_layer) {
                const data_image& _image = images[_vulkan_layer_to_lucaria_face[_layer]];
                const std::vector<uint8> _pixels = _make_upload_pixels(_image);
                if (_pixels.empty()) {
                    continue;
                }
                rendering_vulkan_upload_image(cubemap.image, { _image.width, _image.height }, _pixels.data(), static_cast<VkDeviceSize>(_pixels.size()), VK_IMAGE_ASPECT_COLOR_BIT, cubemap.layout, _layer, 6);
            }
            cubemap.descriptor = rendering_vulkan_add_imgui_texture(cubemap.sampler, cubemap.image_view, cubemap.layout);
        }

    }

    rendering_cubemap::~rendering_cubemap()
    {
        if (_ownership.owns()) {
            rendering_vulkan_remove_imgui_texture(descriptor);
            rendering_vulkan_destroy_sampler(sampler);
            rendering_vulkan_destroy_image(image, memory, image_view);
        }
    }

    rendering_cubemap::rendering_cubemap(const std::array<data_image, 6>& images)
    {
        _upload_cubemap(*this, images);
        _ownership.emplace();
    }

    rendering_cubemap::rendering_cubemap(const std::array<asset_image, 6>& images)
    {
        std::array<data_image, 6> _images = {};
        for (uint32 _index = 0; _index < 6; ++_index) {
            _images[_index] = images[_index].data;
        }
        _upload_cubemap(*this, _images);
        _ownership.emplace();
    }

}
}

#endif
