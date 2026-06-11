#include <lucaria/core/rendering_framebuffer.hpp>

namespace lucaria {
namespace {

    static void _check_complete()
    {
#if defined(LUCARIA_DEBUG)
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LUCARIA_DEBUG_ERROR("Framebuffer incomplete")
        }
#endif
    }

}

namespace detail {

    
    object_framebuffer::~object_framebuffer()
    {
        if (ownership.owns()) {
            glDeleteFramebuffers(1, &id);
        }
    }

    object_framebuffer::object_framebuffer()
    {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        GLenum _none = GL_NONE;
        glDrawBuffers(1, &_none);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
        ownership.emplace();
    }

    void object_framebuffer::use_default()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void object_framebuffer::use()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void object_framebuffer::bind_color(const object_texture& color)
    {
        const GLuint _texture_handle = static_cast<GLuint>(color.id);

        if (texture_color_id && texture_color_id.value() == _texture_handle) {
            return;
        }

        const GLenum _attachment = GL_COLOR_ATTACHMENT0;
        const GLuint _color_id = _texture_handle;
        texture_color_id = _color_id;
        renderbuffer_color_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_id, 0);
        glDrawBuffers(1, &_attachment);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void object_framebuffer::bind_color(object_renderbuffer& color)
    {
        if (renderbuffer_color_id && renderbuffer_color_id.value() == color.id) {
            return;
        }

        const GLenum _attachment = GL_COLOR_ATTACHMENT0;
        const GLuint _color_id = color.id;
        renderbuffer_color_id = _color_id;
        texture_color_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color_id);
        glDrawBuffers(1, &_attachment);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void object_framebuffer::bind_depth(object_texture& depth)
    {
        if (texture_depth_id && texture_depth_id.value() == depth.id) {
            return;
        }

        const GLuint _depth_id = static_cast<GLuint>(depth.id);
        texture_depth_id = _depth_id;
        renderbuffer_depth_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth_id, 0);

        if (!texture_color_id && !renderbuffer_color_id) {
            GLenum _none = GL_NONE;
            glDrawBuffers(1, &_none);
            glReadBuffer(GL_NONE);
        }

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void object_framebuffer::bind_depth(object_renderbuffer& depth)
    {
        if (renderbuffer_depth_id && renderbuffer_depth_id.value() == depth.id) {
            return;
        }

        const GLuint _depth_id = depth.id;
        renderbuffer_depth_id = _depth_id;
        texture_depth_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_id);

        if (!texture_color_id && !renderbuffer_color_id) {
            GLenum _none = GL_NONE;
            glDrawBuffers(1, &_none);
            glReadBuffer(GL_NONE);
        }

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
}