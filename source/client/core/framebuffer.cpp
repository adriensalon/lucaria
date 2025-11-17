#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/opengl.hpp>

namespace lucaria {
namespace {

    static void check_complete()
    {
#if LUCARIA_DEBUG
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LUCARIA_RUNTIME_ERROR("Framebuffer incomplete")
        }
#endif
    }

}

framebuffer::framebuffer(framebuffer&& other)
{
    *this = std::move(other);
}

framebuffer& framebuffer::operator=(framebuffer&& other)
{
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _size = other._size;
    _handle = other._handle;
    _texture_color_id = other._texture_color_id;
    _texture_depth_id = other._texture_depth_id;
    _renderbuffer_color_id = other._renderbuffer_color_id;
    _renderbuffer_depth_id = other._renderbuffer_depth_id;
    other._is_owning = false;
    return *this;
}

framebuffer::~framebuffer()
{
    if (_is_owning) {
        glDeleteFramebuffers(1, &_handle);
    }
}

framebuffer::framebuffer(const glm::uvec2& size)
    : _size(size)
    , _texture_color_id(std::nullopt)
    , _texture_depth_id(std::nullopt)
    , _renderbuffer_color_id(std::nullopt)
    , _renderbuffer_depth_id(std::nullopt)
{
    glGenFramebuffers(1, &_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);
    GLenum _none = GL_NONE;
    glDrawBuffers(1, &_none);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _is_owning = true;
}

void framebuffer::use_default()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);
}

void framebuffer::bind_color(texture& color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint _color_id = color.get_handle();
    _texture_color_id = _color_id;
    _renderbuffer_color_id = std::nullopt;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_id, 0);

    const GLenum buf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_color(renderbuffer& color)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint _color_id = color.get_handle();
    _renderbuffer_color_id = _color_id;
    _texture_color_id = std::nullopt;

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color_id);

    const GLenum buf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_depth(texture& depth)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint _depth_id = depth.get_handle();
    _texture_depth_id = _depth_id;
    _renderbuffer_depth_id = 0;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth_id, 0);

    if (!_texture_color_id && !_renderbuffer_color_id) {
        GLenum _none = GL_NONE;
        glDrawBuffers(1, &_none);
        glReadBuffer(GL_NONE);
    }

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_depth(renderbuffer& depth)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint _depth_id = depth.get_handle();
    _renderbuffer_depth_id = _depth_id;
    _texture_depth_id = 0;

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_id);

    if (!_texture_color_id && !_renderbuffer_color_id) {
        GLenum _none = GL_NONE;
        glDrawBuffers(1, &_none);
        glReadBuffer(GL_NONE);
    }

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::uvec2 framebuffer::get_size() const
{
    return _size;
}

glm::uint framebuffer::get_handle() const
{
    return _handle;
}

}