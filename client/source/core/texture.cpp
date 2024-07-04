#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <core/texture.hpp>
#include <glue/fetch.hpp>

namespace detail {

void validate_texture(const texture_data& data)
{
}

static std::unordered_map<std::string, std::promise<texture_ref>> promises;

}

texture_ref::texture_ref(texture_ref&& other)
{
    *this = std::move(other);
}

texture_ref& texture_ref::operator=(texture_ref&& other)
{
    _texture_id = other._texture_id;
    _must_destroy = true;
    other._must_destroy = false;
    return *this;
}

texture_ref::~texture_ref()
{
    if (_must_destroy) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &_texture_id);
    }
}

texture_ref::texture_ref(const texture_data& data)
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
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
    _must_destroy = true;
}

GLuint texture_ref::get_id() const
{
    return _texture_id;
}

texture_ref load_texture(const std::filesystem::path& file)
{
#if LUCARIA_DEBUG
    if (!std::filesystem::is_regular_file(file)) {
        std::cout << "Invalid texture path " << file << std::endl;
        std::terminate();
    }
#endif
    texture_data _data;
    {
#if LUCARIA_JSON
        std::ifstream _fstream(file);
        cereal::JSONInputArchive _archive(_fstream);
#else
        std::ifstream _fstream(file, std::ios::binary);
        cereal::PortableBinaryInputArchive _archive(_fstream);
#endif
        _archive(_data);
    }
#if LUCARIA_DEBUG
    std::cout << "Loaded texture data from " << file << " ("
              << _data.width << "x"
              << _data.height << ", "
              << _data.channels << " channels)" << std::endl;
#endif
    return texture_ref(_data);
}

std::future<texture_ref> fetch_texture(const std::filesystem::path& file)
{
    std::promise<texture_ref>& _promise = detail::promises[file.generic_string()];
    fetch_file(file.generic_string(), [&_promise, file](std::istringstream& stream) {
        texture_data _data;
        {
#if LUCARIA_JSON
            cereal::JSONInputArchive _archive(stream);
#else
            cereal::PortableBinaryInputArchive _archive(stream);
#endif
            _archive(_data);
        }
#if LUCARIA_DEBUG
        std::cout << "Loaded texture data from " << file << " ("
              << _data.width << "x"
              << _data.height << ", "
              << _data.channels << " channels)" << std::endl;
#endif
        _promise.set_value(std::move(texture_ref(_data)));
    });
    return _promise.get_future();
}