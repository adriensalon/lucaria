#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_texture.hpp>

namespace lucaria {
namespace detail {
    namespace {

        [[nodiscard]] static uint32 _next_pow2(uint32 value)
        {
            uint32 _result = 1;
            while (_result < value) {
                _result <<= 1;
            }
            return _result;
        }

        [[nodiscard]] static bool _is_compressed(const data_image_profile profile)
        {
            return profile == data_image_profile::s3tc_rgb4
                || profile == data_image_profile::s3tc_rgba8;
        }

        [[nodiscard]] static int _texture_psm(const data_image_profile profile, const uint32 channels)
        {
            switch (profile) {
            case data_image_profile::rgb565:
                return GU_PSM_5650;
            case data_image_profile::rgba5551:
                return GU_PSM_5551;
            case data_image_profile::rgba4444:
                return GU_PSM_4444;
            case data_image_profile::s3tc_rgb4:
                return GU_PSM_DXT1;
            case data_image_profile::s3tc_rgba8:
                return channels <= 3 ? GU_PSM_DXT1 : GU_PSM_DXT5;
            default:
                return GU_PSM_8888;
            }
        }

        [[nodiscard]] static uint32 _texture_bytes_per_pixel(const data_image_profile profile)
        {
            switch (profile) {
            case data_image_profile::rgb565:
            case data_image_profile::rgba5551:
            case data_image_profile::rgba4444:
                return 2;
            default:
                return 4;
            }
        }

        [[nodiscard]] static uint32 _source_bytes_per_pixel(const data_image& image)
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

        static void _copy_pixels(rendering_texture& texture, const data_image& image)
        {
            if (_is_compressed(image.profile)) {
                if (texture.pixels == nullptr || image.pixels.empty()) {
                    return;
                }
                std::memcpy(texture.pixels, image.pixels.data(), image.pixels.size());
                sceKernelDcacheWritebackInvalidateAll();
                return;
            }

            const uint32 _destination_bpp = _texture_bytes_per_pixel(texture.profile);
            const uint32 _source_bpp = _source_bytes_per_pixel(image);
            if (texture.pixels == nullptr || image.width == 0 || image.height == 0) {
                return;
            }
            if (_source_bpp == 0 || _destination_bpp == 0) {
                LUCARIA_DEBUG_ERROR("Invalid PSP texture pixel format")
                return;
            }
            const std::size_t _required_bytes = static_cast<std::size_t>(image.width) * image.height * _source_bpp;
            if (image.pixels.size() < _required_bytes) {
                LUCARIA_DEBUG_ERROR("Texture pixel data is smaller than its dimensions and profile require")
                return;
            }
            for (uint32 _y = 0; _y < image.height; ++_y) {
                uint8* _destination = static_cast<uint8*>(texture.pixels) + static_cast<std::size_t>(_y) * texture.tbw * _destination_bpp;
                const uint8* _source = image.pixels.data() + static_cast<std::size_t>(_y) * image.width * _source_bpp;
                if (image.profile == data_image_profile::rgba8888 && image.channels == 3) {
                    for (uint32 _x = 0; _x < image.width; ++_x) {
                        _destination[_x * 4 + 0] = _source[_x * 3 + 0];
                        _destination[_x * 4 + 1] = _source[_x * 3 + 1];
                        _destination[_x * 4 + 2] = _source[_x * 3 + 2];
                        _destination[_x * 4 + 3] = 255;
                    }
                } else {
                    if (_source_bpp != _destination_bpp) {
                        LUCARIA_DEBUG_ERROR("Texture pixel data cannot be converted to its PSP storage profile")
                        return;
                    }
                    std::memcpy(_destination, _source, static_cast<std::size_t>(image.width) * _destination_bpp);
                }
            }
            sceKernelDcacheWritebackInvalidateAll();
        }

        static void _allocate(
            rendering_texture& texture,
            const uint32x2 size,
            const data_image_profile profile,
            const uint32 channels = 4,
            const std::size_t compressed_bytes = 0)
        {
            if (texture.pixels != nullptr) {
                std::free(texture.pixels);
                texture.pixels = nullptr;
            }
            texture.profile = profile;
            texture.size = size;
            texture.psm = _texture_psm(profile, channels);
            texture.tbw = static_cast<int>(_next_pow2(std::max<uint32>(size.x, 1)));
            texture.texture_capacity = {
                static_cast<uint32>(texture.tbw),
                _next_pow2(std::max<uint32>(size.y, 1))
            };
            const std::size_t _bytes = _is_compressed(profile)
                ? compressed_bytes
                : static_cast<std::size_t>(texture.texture_capacity.x) * texture.texture_capacity.y * _texture_bytes_per_pixel(profile);
            if (_bytes == 0) {
                LUCARIA_DEBUG_ERROR("Invalid PSP texture allocation size")
                return;
            }
            texture.pixels = memalign(16, _bytes);
            if (texture.pixels == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to allocate PSP texture")
                return;
            }
            std::memset(texture.pixels, 0, _bytes);
            texture.is_dedicated_storage = true;
            texture.uv_rect = { 0.f, 0.f, 1.f, 1.f };
            sceKernelDcacheWritebackInvalidateAll();
        }

    }

    rendering_textures_registry::~rendering_textures_registry() = default;

    void rendering_textures_registry::upload(rendering_texture& texture, const data_image& image)
    {
        _allocate(texture, { image.width, image.height }, image.profile, image.channels, image.pixels.size());
        _copy_pixels(texture, image);
    }

    void rendering_textures_registry::release(rendering_texture&)
    {
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
        if (pixels != nullptr) {
            std::free(pixels);
            pixels = nullptr;
        }
        texture_capacity = {};
        tbw = 0;
    }

    rendering_texture::rendering_texture(rendering_textures_registry& registry, const data_image& from)
        : profile(from.profile)
        , size(from.width, from.height)
        , _registry(&registry)
    {
        _allocate(*this, size, profile, from.channels, from.pixels.size());
        _copy_pixels(*this, from);
        _ownership.emplace();
    }

    rendering_texture::rendering_texture(const uint32x2 size)
        : profile(data_image_profile::rgba8888)
        , size(size)
    {
        _allocate(*this, size, profile);
        _ownership.emplace();
    }

    void rendering_texture::resize(const uint32x2 new_size)
    {
        if (size == new_size) {
            return;
        }
        _allocate(*this, new_size, data_image_profile::rgba8888);
    }

    void rendering_texture::update(const data_image& from)
    {
        if (size != uint32x2(from.width, from.height) || profile != from.profile || pixels == nullptr) {
            _allocate(*this, { from.width, from.height }, from.profile, from.channels, from.pixels.size());
        }
        _copy_pixels(*this, from);
    }

    ImTextureID rendering_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(pixels);
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
