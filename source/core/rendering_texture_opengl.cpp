#include <lucaria/core/app_error.hpp>
#include <lucaria/core/rendering_texture.hpp>

#include <utility>

namespace lucaria {
namespace detail {
    namespace {

        // constexpr static GLenum COMPRESSED_R11_EAC = 0x9270;
        // constexpr static GLenum COMPRESSED_SIGNED_R11_EAC = 0x9271;
        // constexpr static GLenum COMPRESSED_RG11_EAC = 0x9272;
        // constexpr static GLenum COMPRESSED_SIGNED_RG11_EAC = 0x9273;
        // constexpr static GLenum COMPRESSED_SRGB8_ETC2 = 0x9275;
        // constexpr static GLenum COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276;
        // constexpr static GLenum COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277;
        // constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
        // constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
        // constexpr static GLenum COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;

        constexpr static GLenum COMPRESSED_RGB8_ETC2 = 0x9274;
        constexpr static GLenum COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
        constexpr static GLenum COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
        constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

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

        static void _configure_texture(const GLuint texture_id)
        {
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        static void _upload_dedicated_texture(const GLuint texture_id, const data_image& from)
        {
            _configure_texture(texture_id);
            const GLsizei _pixels_count = static_cast<GLsizei>(from.pixels.size());
            const GLubyte* _pixels_ptr = from.pixels.data();
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            switch (from.channels) {
            case 3:
                if (from.profile == data_image_profile::etc2_rgb4) {
                    glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.width, from.height, 0, _pixels_count, _pixels_ptr);
                } else if (from.profile == data_image_profile::s3tc_rgb4 || from.profile == data_image_profile::s3tc_rgba8) {
                    glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
                } else {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.width, from.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
                }
                break;
            case 4:
                if (from.profile == data_image_profile::etc2_rgba8) {
                    glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.width, from.height, 0, _pixels_count, _pixels_ptr);
                } else if (from.profile == data_image_profile::s3tc_rgba8) {
                    glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
                } else {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.width, from.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
                }
                break;
            default:
                LUCARIA_DEBUG_ERROR("Invalid texture channels count, must be 3 or 4")
                break;
            }
            LUCARIA_DEBUG_OPENGL_ASSERT
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
        if (is_dedicated_storage) {
            if (texture_id != 0) {
                glDeleteTextures(1, &texture_id);
            }
        } else if (_registry != nullptr) {
            _registry->release(*this);
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
            glGenTextures(1, &texture_id);
            _upload_dedicated_texture(texture_id, from);
        }
        _ownership.emplace();
    }

    rendering_texture::rendering_texture(const uint32x2 size)
        : profile(data_image_profile::rgba8888)
        , size(size)
    {
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        _ownership.emplace();
    }

    void rendering_texture::resize(const uint32x2 new_size)
    {
        if (is_dedicated_storage) {
            if (size == new_size) {
                return;
            }
            profile = data_image_profile::rgba8888;
            size = new_size;
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            return;
        }
        _release();
        _registry = nullptr;
        profile = data_image_profile::rgba8888;
        size = new_size;
        is_dedicated_storage = true;
        uv_rect = { 0.f, 0.f, 1.f, 1.f };
        glGenTextures(1, &texture_id);
        _configure_texture(texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    void rendering_texture::update(const data_image& from)
    {
        if (is_dedicated_storage) {
            profile = from.profile;
            size = { from.width, from.height };
            _upload_dedicated_texture(texture_id, from);
            return;
        }
        const bool _use_atlas = _registry != nullptr && _can_atlas_texture(from);
        _release();
        profile = from.profile;
        size = { from.width, from.height };
        if (_use_atlas) {
            is_dedicated_storage = false;
            _registry->upload(*this, from);
        } else {
            is_dedicated_storage = true;
            allocation = {};
            uv_rect = { 0.f, 0.f, 1.f, 1.f };
            glGenTextures(1, &texture_id);
            _upload_dedicated_texture(texture_id, from);
        }
    }

    ImTextureID rendering_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(texture_id));
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
