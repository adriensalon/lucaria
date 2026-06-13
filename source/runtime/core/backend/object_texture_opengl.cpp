#include <lucaria/core/rendering_texture.hpp>
#include <lucaria/core/app_error.hpp>

namespace lucaria {
namespace detail {
    namespace {

        // constexpr static GLenum COMPRESSED_R11_EAC = 0x9270;
        // constexpr static GLenum COMPRESSED_SIGNED_R11_EAC = 0x9271;
        // constexpr static GLenum COMPRESSED_RG11_EAC = 0x9272;
        // constexpr static GLenum COMPRESSED_SIGNED_RG11_EAC = 0x9273;
        constexpr static GLenum COMPRESSED_RGB8_ETC2 = 0x9274;
        // constexpr static GLenum COMPRESSED_SRGB8_ETC2 = 0x9275;
        // constexpr static GLenum COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9276;
        // constexpr static GLenum COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277;
        constexpr static GLenum COMPRESSED_RGBA8_ETC2_EAC = 0x9278;
        // constexpr static GLenum COMPRESSED_SRGB8_ALPHA8_ETC2_EAC = 0x9279;
        constexpr static GLenum COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
        // constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
        // constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
        constexpr static GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

    }

    rendering_texture::~rendering_texture()
    {
        if (_ownership.owns()) {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &id);
        }
    }

    rendering_texture::rendering_texture(const data_image& from)
        : profile(from.profile)
        , size(from.width, from.height)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        const GLsizei _pixels_count = static_cast<GLsizei>(from.pixels.size());
        const GLubyte* _pixels_ptr = from.pixels.data();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        switch (from.channels) {

        case 3:
            if ((from.profile == data_image_profile::etc2_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else if ((from.profile == data_image_profile::s3tc_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.width, from.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        case 4:
            if ((from.profile == data_image_profile::etc2_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else if ((from.profile == data_image_profile::s3tc_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.width, from.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;
        default:
            LUCARIA_DEBUG_ERROR("Invalid texture channels count, must be 3 or 4")
            break;
        }

        _ownership.emplace();
    }

    rendering_texture::rendering_texture(const uint32x2 size)
		// PROFILE ?
        : size(size)
    {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        _ownership.emplace();
    }

    void rendering_texture::resize(const uint32x2 new_size)
    {
        if (size != new_size) {
            size = new_size;
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }
    }

    void rendering_texture::update(const data_image& from)
    {
		profile = from.profile;

        const GLsizei _pixels_count = static_cast<GLsizei>(from.pixels.size());
        const GLubyte* _pixels_ptr = from.pixels.data();

        glBindTexture(GL_TEXTURE_2D, id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        switch (from.channels) {

        case 3:
            if ((from.profile == data_image_profile::etc2_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else if ((from.profile == data_image_profile::s3tc_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.width, from.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        case 4:
            if ((from.profile == data_image_profile::etc2_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else if ((from.profile == data_image_profile::s3tc_compressed)) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.width, from.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.width, from.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        default:
            LUCARIA_DEBUG_ERROR("Invalid texture channels count, must be 3 or 4")
            break;
        }
    }

    ImTextureID rendering_texture::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(id));
    }

}
}