#include <fstream>

#include <core/cubemap.hpp>

namespace detail {

template <typename T>
bool all_equal(T first)
{
    return true;
}

template <typename T, typename... Args>
bool all_equal(T first, Args... args)
{
    return ((first == args) && ...);
}

}

cubemap_ref::cubemap_ref(
    const texture_data& plus_x,
    const texture_data& plus_y,
    const texture_data& plus_z,
    const texture_data& minus_x,
    const texture_data& minus_y,
    const texture_data& minus_z)
{
    glGenTextures(1, &_cubemap_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemap_id);
    GLenum _format;
    if (detail::all_equal(
            plus_x.channels,
            plus_y.channels,
            plus_z.channels,
            minus_x.channels,
            minus_y.channels,
            minus_z.channels,
            3)) {
        _format = GL_RGB;
    } else if (detail::all_equal(
                   plus_x.channels,
                   plus_y.channels,
                   plus_z.channels,
                   minus_x.channels,
                   minus_y.channels,
                   minus_z.channels,
                   4)) {
        _format = GL_RGBA;
    } 
#if DEBUG
    else {
        std::cerr << "Invalid channels across cubemap textures or channels != 3 or channels != 4" << std::endl;
        std::terminate();
    }
#endif
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const GLubyte* _plus_x_ptr = const_cast<const GLubyte*>(plus_x.pixels.data());
    const GLubyte* _plus_y_ptr = const_cast<const GLubyte*>(plus_y.pixels.data());
    const GLubyte* _plus_z_ptr = const_cast<const GLubyte*>(plus_z.pixels.data());
    const GLubyte* _minus_x_ptr = const_cast<const GLubyte*>(minus_x.pixels.data());
    const GLubyte* _minus_y_ptr = const_cast<const GLubyte*>(minus_y.pixels.data());
    const GLubyte* _minus_z_ptr = const_cast<const GLubyte*>(minus_z.pixels.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, _format, plus_x.width, plus_x.height, 0, _format, GL_UNSIGNED_BYTE, _plus_x_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, _format, plus_y.width, plus_y.height, 0, _format, GL_UNSIGNED_BYTE, _plus_y_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, _format, plus_z.width, plus_z.height, 0, _format, GL_UNSIGNED_BYTE, _plus_z_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, _format, minus_x.width, minus_x.height, 0, _format, GL_UNSIGNED_BYTE, _minus_x_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, _format, minus_y.width, minus_y.height, 0, _format, GL_UNSIGNED_BYTE, _minus_y_ptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, _format, minus_z.width, minus_z.height, 0, _format, GL_UNSIGNED_BYTE, _minus_z_ptr);
}

cubemap_ref::~cubemap_ref()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &_cubemap_id);
}

GLuint cubemap_ref::get_id() const
{
    return _cubemap_id;
}