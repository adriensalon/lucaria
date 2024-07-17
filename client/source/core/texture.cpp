#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <GLES3/gl3.h>

#include <core/texture.hpp>
#include <core/fetch.hpp>

namespace detail {

void validate_texture(const texture_data& data)
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
    _is_instanced = true;
}

GLuint texture_ref::get_id() const
{
    return _texture_id;
}

texture_data load_texture_data(std::istringstream& texture_stream)
{
    texture_data _data;
    {
#if LUCARIA_JSON        
        cereal::JSONInputArchive _archive(texture_stream);
#else
        cereal::PortableBinaryInputArchive _archive(texture_stream);
#endif
        _archive(_data);
    }
    return _data;
}

std::shared_future<std::shared_ptr<texture_ref>> fetch_texture(const std::filesystem::path& texture_path)
{
    std::promise<std::shared_ptr<texture_ref>>& _promise = detail::promises[texture_path.string()];
    fetch_file(texture_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<texture_ref>(load_texture_data(stream))));
    });
    return _promise.get_future();
}
