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
        throw std::runtime_error("Framebuffer already owning a GL resource");
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
    // glDrawBuffer(GL_NONE);
    GLenum none = GL_NONE;
    glDrawBuffers(1, &none);

    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);
}

void framebuffer::use_default()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_color(texture& color_tex)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint _color_id = color_tex.get_handle();
    _texture_color_id = _color_id;
    _renderbuffer_color_id = std::nullopt; // switch to texture-backed color

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_id, 0);

    const GLenum buf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_color(deprecated_renderbuffer& color_rb)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint rbId = color_rb.get_id();
    _renderbuffer_color_id = rbId;
    _texture_color_id = std::nullopt; // switch to RB-backed color

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbId);

    const GLenum buf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_depth(texture& depth_tex)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint texId = depth_tex.get_handle();
    _texture_depth_id = texId;
    _renderbuffer_depth_id = 0;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);

    if (!_texture_color_id && !_renderbuffer_color_id) {
        // glDrawBuffer(GL_NONE);
        GLenum none = GL_NONE;
        glDrawBuffers(1, &none);
        glReadBuffer(GL_NONE);
    }

    check_complete();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void framebuffer::bind_depth(deprecated_renderbuffer& depth_rb)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _handle);

    const GLuint rbId = depth_rb.get_id();
    _renderbuffer_depth_id = rbId;
    _texture_depth_id = 0;

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbId);

    if (!_texture_color_id && !_renderbuffer_color_id) {
        // glDrawBuffer(GL_NONE);
        GLenum none = GL_NONE;
        glDrawBuffers(1, &none);
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