#include <fstream>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/graphics.hpp>
#include <lucaria/core/load.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/window.hpp>

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

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<texture_ref>>> promises;

}

texture_ref::texture_ref(texture_ref&& other)
{
    *this = std::move(other);
}

texture_ref& texture_ref::operator=(texture_ref&& other)
{
    _texture_id = other._texture_id;
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

texture_ref::~texture_ref()
{
    if (_is_instanced) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &_texture_id);
    }
}

texture_ref::texture_ref(const image_data& data)
{
    glGenTextures(1, &_texture_id);
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLubyte* _pixels_ptr = &(data.pixels[0]);
    switch (data.channels) {
    case 3:
        if (data.is_compressed_etc && get_is_etc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB8_ETC2, data.width, data.height, 0, data.pixels.size(), _pixels_ptr);
        } else if (data.is_compressed_s3tc && get_is_s3tc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, data.width, data.height, 0, data.pixels.size(), _pixels_ptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
        }
        break;
    case 4:
        if (data.is_compressed_etc && get_is_etc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA8_ETC2_EAC, data.width, data.height, 0, data.pixels.size(), _pixels_ptr);
        } else if (data.is_compressed_s3tc && get_is_s3tc_supported()) {
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGBA_S3TC_DXT5_EXT, data.width, data.height, 0, data.pixels.size(), _pixels_ptr);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
        }
        break;
    default:
#if LUCARIA_DEBUG
        std::cout << "Invalid channels count, must be 3 or 4" << std::endl;
        std::terminate();
#endif
        break;
    }
#if LUCARIA_DEBUG
    std::cout << "Created TEXTURE_2D buffer of size " << data.width << "x" << data.height << " with id " << _texture_id << std::endl;
#endif
    _is_instanced = true;
}

GLuint texture_ref::get_id() const
{
    return _texture_id;
}

image_data load_image_data(const std::vector<char>& image_bytes)
{
    image_data _data;
    {
        raw_input_stream _stream(image_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(_data);
    }

    return _data;
}

image_data load_compressed_image_data(const std::vector<char>& image_bytes)
{
    image_data _image_data;
    const std::vector<uint8_t>& _content = *(reinterpret_cast<const std::vector<uint8_t>*>(&image_bytes));
    // try_parse_compressed_texture(_content.data(), _content.size(), _data);
    uint32_t* data32 = (uint32_t*)_content.data();
    if (*data32 == 0x03525650) {
        // PVR
        switch (*(data32 + 2)) {
        // case 6:
        //     // m_type = Etc1;
        //     std::cout << "ETC1" << std::endl;
        //     break;
        case 7:
            // m_type = Dxt1;
            // std::cout << "DXT1" << std::endl;
            _image_data.channels = 3;
            _image_data.is_compressed_s3tc = true;
            break;
        case 11:
            // m_type = Dxt5;
            // std::cout << "DXT5" << std::endl;
            _image_data.channels = 4;
            _image_data.is_compressed_s3tc = true;
            break;
        case 22:
            // m_type = Etc2_RGB;
            std::cout << "ETC2_RGB" << std::endl;
            _image_data.channels = 3;
            _image_data.is_compressed_etc = true;
            break;
        case 23:
            // m_type = Etc2_RGBA;
            std::cout << "ETC2_RGBA" << std::endl;
            _image_data.channels = 4;
            _image_data.is_compressed_etc = true;
            break;
        default:
            assert(false);
            break;
        }
        const std::size_t _offset = 52 + *(data32 + 12);
        const glm::uint8* _data_ptr = _content.data() + _offset;
        const std::size_t _data_size = _content.size() - _offset;
        _image_data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
        _image_data.height = *(data32 + 6);
        _image_data.width = *(data32 + 7);
    } else if (*data32 == 0x58544BAB) {
        // KTX
        switch (*(data32 + 7)) {
        case 0x9274:
            // m_type = Etc2_RGB;
            std::cout << "ETC2_RGB" << std::endl;
            _image_data.channels = 3;
            _image_data.is_compressed_etc = true;
            break;
        case 0x9278:
            // m_type = Etc2_RGBA;
            std::cout << "ETC2_RGBA" << std::endl;
            _image_data.channels = 4;
            _image_data.is_compressed_etc = true;
            break;
        default:
            assert(false);
            break;
        }
        const std::size_t _offset = sizeof(uint32_t) * 17 + *(data32 + 15);
        const glm::uint8* _data_ptr = _content.data() + _offset;
        const std::size_t _data_size = _content.size() - _offset;
        _image_data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
        _image_data.width = *(data32 + 9);
        _image_data.height = *(data32 + 10);
    } else {
        assert(false);
    }
    return _image_data;
}

std::shared_future<std::shared_ptr<texture_ref>> fetch_texture(const std::filesystem::path& image_path, const std::optional<std::filesystem::path>& etc_image_path, const std::optional<std::filesystem::path>& s3tc_image_path)
{
    bool _is_compressed;
    std::filesystem::path _image_path;
    if (get_is_etc_supported() && etc_image_path.has_value()) {
        _is_compressed = true;
        _image_path = etc_image_path.value();
    } else if (get_is_s3tc_supported() && s3tc_image_path.has_value()) {
        _is_compressed = true;
        _image_path = s3tc_image_path.value();
    } else {
        _is_compressed = false;
        _image_path = image_path;
    }
    std::promise<std::shared_ptr<texture_ref>>& _promise = detail::promises[_image_path.string()];
    if (_is_compressed) {
        fetch_file(_image_path, [&_promise](const std::vector<char>& image_bytes) {
            _promise.set_value(std::make_shared<texture_ref>(load_compressed_image_data(image_bytes)));
        });
    } else {
        fetch_file(_image_path, [&_promise](const std::vector<char>& image_bytes) {
            _promise.set_value(std::make_shared<texture_ref>(load_image_data(image_bytes)));
        });
    }

    return _promise.get_future();
}

void clear_texture_fetches()
{
    detail::promises.clear();
}