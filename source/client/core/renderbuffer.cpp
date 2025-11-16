#include <lucaria/core/opengl.hpp>
#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {

deprecated_renderbuffer::deprecated_renderbuffer(deprecated_renderbuffer&& other)
{
    *this = std::move(other);
}

deprecated_renderbuffer& deprecated_renderbuffer::operator=(deprecated_renderbuffer&& other)
{
    if (_is_instanced) {
        throw std::runtime_error("Renderbuffer already owning a GL resource");
    }
    _is_instanced = true;
    _width = other._width;
    _height = other._height;
    _internal_format = other._internal_format;
    _samples = other._samples;
    _renderbuffer_id = other._renderbuffer_id;
    other._is_instanced = false;
    return *this;
}

deprecated_renderbuffer::~deprecated_renderbuffer()
{
    if (_is_instanced) {
        glDeleteRenderbuffers(1, &_renderbuffer_id);
    }
}

deprecated_renderbuffer::deprecated_renderbuffer(const glm::uint width, const glm::uint height, const glm::uint internal_format, const glm::uint samples)
{
    _width = width;
    _height = height;
    _internal_format = internal_format;
    _samples = samples;

    glGenRenderbuffers(1, &_renderbuffer_id);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer_id);

    GLint _max_samples = 1;
#ifdef GL_MAX_SAMPLES
    glGetIntegerv(GL_MAX_SAMPLES, &_max_samples);
#endif
    _samples = static_cast<glm::uint>(std::clamp<int>(_samples, 1, _max_samples));

#if defined(GL_RENDERBUFFER_SAMPLES) // desktop GL / GLES3
    if (_samples > 1) {
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            static_cast<GLsizei>(_samples),
            _internal_format,
            static_cast<GLsizei>(_width),
            static_cast<GLsizei>(_height));
    } else
#endif
    {
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            _internal_format,
            static_cast<GLsizei>(_width),
            static_cast<GLsizei>(_height));
    }
}

glm::uint deprecated_renderbuffer::get_width() const
{
    return _width;
}

glm::uint deprecated_renderbuffer::get_height() const
{
    return _height;
}

glm::uint deprecated_renderbuffer::get_internal_format() const
{
    return _internal_format;
}

glm::uint deprecated_renderbuffer::get_samples() const
{
    return _samples;
}

glm::uint deprecated_renderbuffer::get_id() const
{
    return _renderbuffer_id;
}

}