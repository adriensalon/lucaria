#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/error.hpp>
#include <lucaria/core/opengl.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {
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

texture::texture(texture&& other)
{
    *this = std::move(other);
}

texture& texture::operator=(texture&& other)
{
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _size = other._size;
    _handle = other._handle;
    other._is_owning = false;
    return *this;
}

texture::~texture()
{
    if (_is_owning) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &_handle);
    }
}

texture::texture(const image& from)
{
    _size = { from.data.width, from.data.height };
    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLsizei _pixels_count = static_cast<GLsizei>(from.data.pixels.size());
    const GLubyte* _pixels_ptr = from.data.pixels.data();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    switch (from.data.channels) {
    case 3:
        if (from.data.is_compressed_etc && get_is_etc2_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
        } else if (from.data.is_compressed_s3tc && get_is_s3tc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, from.data.width, from.data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
        }
        break;
    case 4:
        if (from.data.is_compressed_etc && get_is_etc2_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
        } else if (from.data.is_compressed_s3tc && get_is_s3tc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, from.data.width, from.data.height, 0, _pixels_count, _pixels_ptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, from.data.width, from.data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
        }
        break;
    default:
        LUCARIA_RUNTIME_ERROR("Invalid texture channels count, must be 3 or 4")
        break;
    }
    LUCARIA_RUNTIME_OPENGL_ASSERT
#if LUCARIA_DEBUG
    std::cout << "Created TEXTURE_2D buffer of size " << from.data.width << "x" << from.data.height << " with id " << _handle << std::endl;
#endif
    _is_owning = true;
}

texture::texture(const glm::uvec2 size)
{
    _size = size;
    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Allocate storage without data
    glTexImage2D(
        GL_TEXTURE_2D,
        0, // mipmap level
        GL_RGBA, // internal format (choose GL_RGB if you don’t need alpha)
        size.x,
        size.y,
        0, // border (must be 0)
        GL_RGBA, // format
        GL_UNSIGNED_BYTE, // type
        0);

#if LUCARIA_DEBUG
    std::cout << "Created EMPTY TEXTURE_2D buffer of size "
              << size.x << "x" << size.y
              << " with id " << _handle << std::endl;
#endif

    _is_owning = true;
}

void texture::resize(const glm::uvec2 size)
{
    if (_size != size) {
        _size = size;
        glBindTexture(GL_TEXTURE_2D, _handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
}

glm::uvec2 texture::get_size() const
{
    return _size;
}

glm::uint texture::get_handle() const
{
    return _handle;
}

fetched<texture> fetch_texture(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path,
    const std::optional<std::filesystem::path>& s3tc_path)
{
    const std::filesystem::path& _image_path = detail::resolve_image_path(data_path, etc2_path, s3tc_path);
    std::shared_ptr<std::promise<image>> _image_promise = std::make_shared<std::promise<image>>();

    detail::fetch_bytes(_image_path, [_image_promise](const std::vector<char>& _data_bytes) {
        image _image(_data_bytes);
        _image_promise->set_value(std::move(_image));
    });

    // create texture on main thread
    return fetched<texture>(_image_promise->get_future(), [](const image& _from) {
        return texture(_from);
    });
}

}
