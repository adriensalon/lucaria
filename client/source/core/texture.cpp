#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <core/texture.hpp>

namespace lucaria {

texture::texture(const std::filesystem::path& file)
{
    std::ifstream _fstream(file);
    cereal::PortableBinaryInputArchive _archive(_fstream);
    unsigned int _width, _height, _channels;
    std::vector<unsigned char> _pixels;
    _archive(_channels);
    _archive(_width);
    _archive(_height);
    _archive(_pixels);
    glGenTextures(1, &_texture_id);
    glBindTexture(GL_TEXTURE_2D, _texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLubyte* _pixels_ptr = &(_pixels[0]);
    switch (_channels) {
    case 3:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels_ptr);
        break;
    case 4:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels_ptr);
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

texture::~texture()
{

}

GLuint texture::get_id() const
{
    return _texture_id;
}

}