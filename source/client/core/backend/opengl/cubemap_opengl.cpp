#include <lucaria/core/cubemap.hpp>
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

        const std::array<GLenum, 6> cubemap_enums = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

    }

    cubemap_implementation::~cubemap_implementation()
    {
        if (implementation_opengl.ownership.owns()) {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glDeleteTextures(1, &implementation_opengl.id);
        }
    }

    cubemap_implementation::cubemap_implementation(const std::array<detail::image_implementation, 6>& images)
    {
        glGenTextures(1, &implementation_opengl.id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, implementation_opengl.id);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        for (glm::uint _index = 0; _index < 6; ++_index) {
            const detail::image_implementation& _image = images[_index];
            const GLsizei _pixels_count = static_cast<GLsizei>(_image.data.pixels.size());
            const GLubyte* _pixels_ptr = _image.data.pixels.data();
            const GLenum _side_enum = cubemap_enums[_index];
            switch (_image.data.channels) {
            case 3:
                if (_image.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                    glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB8_ETC2, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
                } else if (_image.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                    glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
                } else {
                    glTexImage2D(_side_enum, 0, GL_RGB, _image.data.width, _image.data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
                }
                break;
            case 4:
                if (_image.data.is_compressed_etc && detail::engine_window().is_etc2_supported) {
                    glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGBA8_ETC2_EAC, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
                } else if (_image.data.is_compressed_s3tc && detail::engine_window().is_s3tc_supported) {
                    glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
                } else {
                    glTexImage2D(_side_enum, 0, GL_RGBA, _image.data.width, _image.data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
                }
                break;
            default:
                LUCARIA_RUNTIME_ERROR("Invalid channels count, must be 3 or 4")
                break;
            }
        }
		
        implementation_opengl.ownership.emplace();
    }

}

}