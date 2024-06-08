#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <core/texture.hpp>

texture_ref::texture_ref(const texture_data& data)
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, data.width, data.height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data.width, data.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
        break;
    default:
#if DEBUG
        std::cerr << "Invalid channels count, must be 3 or 4" << std::endl;
        std::terminate();
#else
        break;
#endif
    }
}

texture_ref::~texture_ref()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &_texture_id);
}

GLuint texture_ref::get_id() const
{
    return _texture_id;
}

texture_data load_texture(const std::filesystem::path& file)
{
    texture_data _data;
    std::ifstream _fstream(file);
    cereal::PortableBinaryInputArchive _archive(_fstream);
    _archive(_data.channels);
    _archive(_data.width);
    _archive(_data.height);
    _archive(_data.pixels);
    return _data;
}