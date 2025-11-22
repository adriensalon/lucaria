#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/error.hpp>

#if LUCARIA_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#elif LUCARIA_PLATFORM_WEB
#include <GLES3/gl3.h>
#elif LUCARIA_PLATFORM_WIN32
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

namespace lucaria {

extern bool _is_etc2_supported;
extern bool _is_s3tc_supported;
extern const std::filesystem::path& _resolve_image_path(const std::filesystem::path& data_path, const std::optional<std::filesystem::path>& etc2_path, const std::optional<std::filesystem::path>& s3tc_path);
extern std::vector<std::filesystem::path> _resolve_image_paths(const std::array<std::filesystem::path, 6>& data_paths, const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths, const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths);
extern void _fetch_bytes(const std::vector<std::filesystem::path>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist);

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

cubemap::cubemap(cubemap&& other)
{
    *this = std::move(other);
}

cubemap& cubemap::operator=(cubemap&& other)
{
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _handle = other._handle;
    other._is_owning = false;
    return *this;
}

cubemap::~cubemap()
{
    if (_is_owning) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        glDeleteTextures(1, &_handle);
    }
}

cubemap::cubemap(const std::array<image, 6>& images)
{
    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for (glm::uint _index = 0; _index < 6; ++_index) {
        const image& _image = images[_index];
        const GLsizei _pixels_count = static_cast<GLsizei>(_image.data.pixels.size());
        const GLubyte* _pixels_ptr = _image.data.pixels.data();
        const GLenum _side_enum = cubemap_enums[_index];
        switch (_image.data.channels) {
        case 3:
            if (_image.data.is_compressed_etc && _is_etc2_supported) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB8_ETC2, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (_image.data.is_compressed_s3tc && _is_s3tc_supported) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
            } else {
                glTexImage2D(_side_enum, 0, GL_RGB, _image.data.width, _image.data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
            }
            break;
        case 4:
            if (_image.data.is_compressed_etc && _is_etc2_supported) {
                glCompressedTexImage2D(_side_enum, 0, COMPRESSED_RGBA8_ETC2_EAC, _image.data.width, _image.data.height, 0, _pixels_count, _pixels_ptr);
            } else if (_image.data.is_compressed_s3tc && _is_s3tc_supported) {
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
}

glm::uint cubemap::get_handle() const
{
    return _handle;
}

fetched<cubemap> fetch_cubemap(
    const std::array<std::filesystem::path, 6>& data_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths)
{
    const std::vector<std::filesystem::path> _image_paths = _resolve_image_paths(data_paths, etc2_paths, s3tc_paths);
    std::shared_ptr<std::promise<std::array<image, 6>>> _images_promise = std::make_shared<std::promise<std::array<image, 6>>>();
    _fetch_bytes(_image_paths, [_images_promise](const std::vector<std::vector<char>>& _data_bytes) {
        std::array<image, 6> _images = {
            image(_data_bytes[0]),
            image(_data_bytes[1]),
            image(_data_bytes[2]),
            image(_data_bytes[3]),
            image(_data_bytes[4]),
            image(_data_bytes[5])
        };
        _images_promise->set_value(std::move(_images)); }, true);

    // create cubemap on main thread
    return fetched<cubemap>(_images_promise->get_future(), [](const std::array<image, 6>& _from) {
        return cubemap(_from);
    });
}

}
