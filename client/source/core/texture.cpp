#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <GLES3/gl3.h>

#include <core/texture.hpp>
#include <core/fetch.hpp>
#include <core/window.hpp>

namespace detail {

void validate_texture(const image_data& data)
{
}

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


const GLenum COMPRESSED_RGB_S3TC_DXT1_EXT        = 0x83F0;
const GLenum COMPRESSED_RGBA_S3TC_DXT1_EXT       = 0x83F1;
const GLenum COMPRESSED_RGBA_S3TC_DXT3_EXT       = 0x83F2;
const GLenum COMPRESSED_RGBA_S3TC_DXT5_EXT       = 0x83F3;

texture_ref::texture_ref(const image_data& data)
{
    detail::validate_texture(data);
    glGenTextures(1, &_texture_id);
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLubyte* _pixels_ptr = &(data.pixels[0]);
    switch (data.channels) {
    case 3:
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, COMPRESSED_RGB_S3TC_DXT1_EXT, data.width, data.height, 0, data.pixels.size(), /* void ptr to unsigned bytes */ _pixels_ptr);
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
        break;
    default:
#if LUCARIA_DEBUG
        std::cout << "Invalid channels count, must be 3 or 4" << std::endl;
        std::terminate();
#endif
        break;
    }
#if LUCARIA_DEBUG
    std::cout << "Created TEXTURE_2D buffer of size " << data.width << "x" << data.height
              << " with id " << _texture_id << std::endl;
#endif
    graphics_assert();
    _is_instanced = true;
}

GLuint texture_ref::get_id() const
{
    return _texture_id;
}

bool try_parse_compressed_texture(uint8_t* data, std::size_t length, glm::uint& width, glm::uint& height, glm::uint& channels, std::size_t& offset) 
{
    auto data32 = (uint32_t*)data;
    if (*data32 == 0x03525650) {
        // PVR
        switch (*(data32 + 2)) {
        // case 6:
        //     // m_type = Etc1;
        //     std::cout << "ETC1" << std::endl;
        //     break;
        case 7:
            // m_type = Dxt1;
            std::cout << "DXT1" << std::endl;
            channels = 3;
            break;
        case 11:
            // m_type = Dxt5;
            std::cout << "DXT5" << std::endl;
            channels = 4;
            break;
        case 22:
            // m_type = Etc2_RGB;
            std::cout << "ETC2_RGB" << std::endl;
            channels = 3;
            break;
        case 23:
            // m_type = Etc2_RGBA;
            std::cout << "ETC2_RGBA" << std::endl;
            channels = 4;
            break;
        default:
            assert(false);
            break;
        }
        height = *(data32 + 6);
        width = *(data32 + 7);
        offset = 52 + *(data32 + 12);
    } else if (*data32 == 0x58544BAB) {
        // KTX
        switch (*(data32 + 7)) {
        case 0x9274:
            // m_type = Etc2_RGB;
            std::cout << "ETC2_RGB" << std::endl;
            channels = 3;
            break;
        case 0x9278:
            // m_type = Etc2_RGBA;
            std::cout << "ETC2_RGBA" << std::endl;
            channels = 4;
            break;
        default:
            assert(false);
            break;
        }
        width = *(data32 + 9);
        height = *(data32 + 10);
        offset = sizeof(uint32_t) * 17 + *(data32 + 15);
    } else {
        return false;
    }
    return true;
}

image_data load_image_data(std::istringstream& texture_stream)
{
    image_data _data;
    std::vector<uint8_t> _content((std::istreambuf_iterator<char>(texture_stream)), std::istreambuf_iterator<char>());
    std::size_t _offset = 0;
    if (try_parse_compressed_texture(_content.data(), _content.size(), _data.width, _data.height, _data.channels, _offset)) {
        std::cout << "width = " << _data.width << std::endl;
        std::cout << "height = " << _data.height << std::endl;
        std::cout << "channels = " << _data.channels << std::endl;

        glm::uint8* _data_ptr = _content.data() + _offset;
        std::size_t _data_size = _content.size() - _offset;
        _data.pixels = std::vector<glm::uint8>(_data_ptr, _data_ptr + _data_size);
        std::cout << "offset = " << _offset << std::endl;
        std::cout << "size = " << _data_size << std::endl;
    }

//     {
// #if LUCARIA_JSON        
//         cereal::JSONInputArchive _archive(texture_stream);
// #else
//         cereal::PortableBinaryInputArchive _archive(texture_stream);
// #endif
//         _archive(_data);
//     }

    return _data;
}

std::shared_future<std::shared_ptr<texture_ref>> fetch_texture(const std::filesystem::path& texture_path)
{
    std::promise<std::shared_ptr<texture_ref>>& _promise = detail::promises[texture_path.string()];
    fetch_file(texture_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<texture_ref>(load_image_data(stream))));
    });
    return _promise.get_future();
}
