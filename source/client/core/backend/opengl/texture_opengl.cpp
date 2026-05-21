#include <lucaria/core/texture.hpp>
#include <lucaria/core/window.hpp>

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

        [[nodiscard]] static texture_origin _convert_origin(const image_origin origin)
        {
            switch (origin) {
            case image_origin::path:
                return texture_origin::path;
            case image_origin::data:
                return texture_origin::data;
            default:
                LUCARIA_RUNTIME_ERROR("Invalid implementation")
                return {};
            }
        }

    }

    texture_implementation::~texture_implementation()
    {
        if (implementation_opengl.ownership.owns()) {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &implementation_opengl.id);
        }
    }

    texture_implementation::texture_implementation(const image_implementation& from)
        : origin(_convert_origin(from.origin))
    {
        size = { from.data.width, from.data.height };
        glGenTextures(1, &implementation_opengl.id);
        glBindTexture(GL_TEXTURE_2D, implementation_opengl.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        const GLsizei _pixels_count = static_cast<GLsizei>(from.data.pixels.size());
        const GLubyte* _pixels_ptr = from.data.pixels.data();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        switch (from.data.channels) {

        case 3:
            if (from.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (from.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.data.width, from.data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        case 4:
            if (from.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (from.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.data.width, from.data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;
        default:
            LUCARIA_RUNTIME_ERROR("Invalid texture channels count, must be 3 or 4")
            break;
        }
        // #if defined(LUCARIA_DEBUG)
        //         std::cout << "Created TEXTURE_2D buffer of size " << from.data.width << "x" << from.data.height << " with id " << implementation_opengl.id << std::endl;
        // #endif

        implementation_opengl.ownership.emplace();
    }

    texture_implementation::texture_implementation(const glm::uvec2 size)
        : origin(texture_origin::size)
        , size(size)
    {
        glGenTextures(1, &implementation_opengl.id);
        glBindTexture(GL_TEXTURE_2D, implementation_opengl.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        // #if defined(LUCARIA_DEBUG)
        //         std::cout << "Created EMPTY TEXTURE_2D buffer of size " << size.x << "x" << size.y << " with id " << implementation_opengl.id << std::endl;
        // #endif

        implementation_opengl.ownership.emplace();
    }

    void texture_implementation::resize(const uint32x2 new_size)
    {
		origin = texture_origin::size;

        if (size != new_size) {
            size = new_size;
            glBindTexture(GL_TEXTURE_2D, implementation_opengl.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }
    }

    void texture_implementation::update(const image_implementation& from)
    {
		origin = texture_origin::data;

        const GLsizei _pixels_count = static_cast<GLsizei>(from.data.pixels.size());
        const GLubyte* _pixels_ptr = from.data.pixels.data();

        glBindTexture(GL_TEXTURE_2D, implementation_opengl.id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        switch (from.data.channels) {

        case 3:
            if (from.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (from.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.data.width, from.data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        case 4:
            if (from.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (from.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.data.width, from.data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;

        default:
            LUCARIA_RUNTIME_ERROR("Invalid texture channels count, must be 3 or 4")
            break;
        }
    }

    ImTextureID texture_implementation::imgui_texture() const
    {
        return reinterpret_cast<ImTextureID>(
            static_cast<std::uintptr_t>(implementation_opengl.id));
    }

}
}