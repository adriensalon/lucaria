#include <lucaria/core/opengl.hpp>
#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {

renderbuffer::renderbuffer(renderbuffer&& other)
{
    *this = std::move(other);
}

renderbuffer& renderbuffer::operator=(renderbuffer&& other)
{
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _size = other._size;
    _handle = other._handle;
    _internal_format = other._internal_format;
    _samples = other._samples;
    other._is_owning = false;
    return *this;
}

renderbuffer::~renderbuffer()
{
    if (_is_owning) {
        glDeleteRenderbuffers(1, &_handle);
    }
}

renderbuffer::renderbuffer(const glm::uvec2& size, const glm::uint internal_format, const glm::uint samples)
{
    _size = size;
    _internal_format = internal_format;
    _samples = samples;

    glGenRenderbuffers(1, &_handle);
    glBindRenderbuffer(GL_RENDERBUFFER, _handle);

    static GLint _max_samples = 1;
#if defined(GL_MAX_SAMPLES)
    if (_max_samples == 1) {
        glGetIntegerv(GL_MAX_SAMPLES, &_max_samples);
    }
#endif
    _samples = static_cast<glm::uint>(std::clamp<int>(_samples, 1, _max_samples));

#if defined(GL_RENDERBUFFER_SAMPLES) // desktop GL / GLES3
    if (_samples > 1) {
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER,
            static_cast<GLsizei>(_samples),
            _internal_format,
            static_cast<GLsizei>(_size.x),
            static_cast<GLsizei>(_size.y));
    } else
#endif
    {
        glRenderbufferStorage(
            GL_RENDERBUFFER,
            _internal_format,
            static_cast<GLsizei>(_size.x),
            static_cast<GLsizei>(_size.y));
    }
    _is_owning = true;
}

glm::uvec2 renderbuffer::get_size() const
{
    return _size;
}

glm::uint renderbuffer::get_handle() const
{
    return _handle;
}

glm::uint renderbuffer::get_internal_format() const
{
    return _internal_format;
}

glm::uint renderbuffer::get_samples() const
{
    return _samples;
}

}