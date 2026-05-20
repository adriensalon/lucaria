#include <lucaria/core/framebuffer.hpp>

namespace lucaria {
namespace {

    static void _check_complete()
    {
#if defined(LUCARIA_DEBUG)
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LUCARIA_RUNTIME_ERROR("Framebuffer incomplete")
        }
#endif
    }

}

namespace detail {

    framebuffer_implementation::framebuffer_implementation(framebuffer_implementation&& other)
    {
        implementation_opengl.is_owning = false;
        *this = std::move(other);
    }

    framebuffer_implementation& framebuffer_implementation::operator=(framebuffer_implementation&& other)
    {
        if (implementation_opengl.is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }
        
		implementation_opengl.is_owning = other.implementation_opengl.is_owning;
		implementation_opengl.id = other.implementation_opengl.id;
        implementation_opengl.texture_color_id = other.implementation_opengl.texture_color_id;
        implementation_opengl.texture_depth_id = other.implementation_opengl.texture_depth_id;
        implementation_opengl.renderbuffer_color_id = other.implementation_opengl.renderbuffer_color_id;
        implementation_opengl.renderbuffer_depth_id = other.implementation_opengl.renderbuffer_depth_id;

        other.implementation_opengl.is_owning = false;
        return *this;
    }

    framebuffer_implementation::~framebuffer_implementation()
    {
        if (implementation_opengl.is_owning) {
            glDeleteFramebuffers(1, &implementation_opengl.id);
        }
    }

    framebuffer_implementation::framebuffer_implementation()
    {
        glGenFramebuffers(1, &implementation_opengl.id);
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
        GLenum _none = GL_NONE;
        glDrawBuffers(1, &_none);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        implementation_opengl.is_owning = true;
    }

    void framebuffer_implementation::use_default()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer_implementation::use()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
    }

    void framebuffer_implementation::bind_color(const texture_implementation& color)
    {
        const GLuint _texture_handle = static_cast<GLuint>(color.implementation_opengl.id);

        if (implementation_opengl.texture_color_id && implementation_opengl.texture_color_id.value() == _texture_handle) {
            return;
        }

        const GLenum _attachment = GL_COLOR_ATTACHMENT0;
        const GLuint _color_id = _texture_handle;
        implementation_opengl.texture_color_id = _color_id;
        implementation_opengl.renderbuffer_color_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_id, 0);
        glDrawBuffers(1, &_attachment);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer_implementation::bind_color(renderbuffer_implementation& color)
    {
        if (implementation_opengl.renderbuffer_color_id && implementation_opengl.renderbuffer_color_id.value() == color.implementation_opengl.id) {
            return;
        }

        const GLenum _attachment = GL_COLOR_ATTACHMENT0;
        const GLuint _color_id = color.implementation_opengl.id;
        implementation_opengl.renderbuffer_color_id = _color_id;
        implementation_opengl.texture_color_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color_id);
        glDrawBuffers(1, &_attachment);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer_implementation::bind_depth(texture_implementation& depth)
    {
        if (implementation_opengl.texture_depth_id && implementation_opengl.texture_depth_id.value() == depth.implementation_opengl.id) {
            return;
        }

        const GLuint _depth_id = static_cast<GLuint>(depth.implementation_opengl.id);
        implementation_opengl.texture_depth_id = _depth_id;
        implementation_opengl.renderbuffer_depth_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depth_id, 0);

        if (!implementation_opengl.texture_color_id && !implementation_opengl.renderbuffer_color_id) {
            GLenum _none = GL_NONE;
            glDrawBuffers(1, &_none);
            glReadBuffer(GL_NONE);
        }

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void framebuffer_implementation::bind_depth(renderbuffer_implementation& depth)
    {
        if (implementation_opengl.renderbuffer_depth_id && implementation_opengl.renderbuffer_depth_id.value() == depth.implementation_opengl.id) {
            return;
        }

        const GLuint _depth_id = depth.implementation_opengl.id;
        implementation_opengl.renderbuffer_depth_id = _depth_id;
        implementation_opengl.texture_depth_id = std::nullopt;
        glBindFramebuffer(GL_FRAMEBUFFER, implementation_opengl.id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_id);

        if (!implementation_opengl.texture_color_id && !implementation_opengl.renderbuffer_color_id) {
            GLenum _none = GL_NONE;
            glDrawBuffers(1, &_none);
            glReadBuffer(GL_NONE);
        }

        _check_complete();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
}