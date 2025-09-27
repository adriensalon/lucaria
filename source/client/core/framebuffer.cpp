// #include <lucaria/core/framebuffer.hpp>
// #include <lucaria/core/opengl.hpp>

// namespace lucaria {
// namespace {

//     static void check_complete()
//     {
// #if LUCARIA_DEBUG
//         GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//         if (status != GL_FRAMEBUFFER_COMPLETE) {
//             LUCARIA_RUNTIME_ERROR("Framebuffer incomplete")
//         }
// #endif
//     }

// }

// framebuffer::framebuffer(framebuffer&& other)
// {
//     *this = std::move(other);
// }

// framebuffer& framebuffer::operator=(framebuffer&& other)
// {
//     if (_is_instanced) {
//         throw std::runtime_error("Framebuffer already owning a GL resource");
//     }
//     _is_instanced = true;
//     _width = other._width;
//     _height = other._height;
//     _framebuffer_id = other._framebuffer_id;
//     _texture_color_id = other._texture_color_id;
//     _texture_depth_id = other._texture_depth_id;
//     _renderbuffer_color_id = other._renderbuffer_color_id;
//     _renderbuffer_depth_id = other._renderbuffer_depth_id;
//     other._is_instanced = false;
//     return *this;
// }

// framebuffer::~framebuffer()
// {
//     if (_is_instanced) {
//         glDeleteFramebuffers(1, &_framebuffer_id);
//     }
// }

// framebuffer::framebuffer(const glm::uint width, const glm::uint height)
// {
//     _width = width;
//     _height = height;
//     _texture_color_id = std::nullopt;
//     _texture_depth_id = std::nullopt;
//     _renderbuffer_color_id = std::nullopt;
//     _renderbuffer_depth_id = std::nullopt;

//     glGenFramebuffers(1, &_framebuffer_id);
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);
//     // glDrawBuffer(GL_NONE);
//     glReadBuffer(GL_NONE);
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// void framebuffer::color(const texture& color_tex)
// {
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);

//     const GLuint _color_id = color_tex.get_id();
//     _texture_color_id = _color_id;
//     _renderbuffer_color_id = std::nullopt; // switch to texture-backed color

//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color_id, 0);

//     const GLenum buf = GL_COLOR_ATTACHMENT0;
//     glDrawBuffers(1, &buf);
//     glReadBuffer(GL_COLOR_ATTACHMENT0);

//     check_complete();
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// void framebuffer::color(const renderbuffer_ref& color_rb)
// {
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);

//     const GLuint rbId = color_rb.get_id();
//     _renderbuffer_color_id = rbId;
//     _texture_color_id = std::nullopt; // switch to RB-backed color

//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbId);

//     const GLenum buf = GL_COLOR_ATTACHMENT0;
//     glDrawBuffers(1, &buf);
//     glReadBuffer(GL_COLOR_ATTACHMENT0);

//     check_complete();
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// void framebuffer::depth(const texture& depth_tex)
// {
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);

//     const GLuint texId = depth_tex.get_id();
//     _texture_depth_id = texId;
//     _renderbuffer_depth_id = 0;

//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texId, 0);

//     if (!_texture_color_id && !_renderbuffer_color_id) {
//         // glDrawBuffer(GL_NONE);
//         glReadBuffer(GL_NONE);
//     }

//     check_complete();
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// void framebuffer::depth(const renderbuffer_ref& depth_rb)
// {
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer_id);

//     const GLuint rbId = depth_rb.get_id();
//     _renderbuffer_depth_id = rbId;
//     _texture_depth_id = 0;

//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbId);

//     if (!_texture_color_id && !_renderbuffer_color_id) {
//         // glDrawBuffer(GL_NONE);
//         glReadBuffer(GL_NONE);
//     }

//     check_complete();
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }

// glm::uint framebuffer::get_width() const
// {
//     return _width;
// }

// glm::uint framebuffer::get_height() const
// {
//     return _height;
// }

// glm::uint framebuffer::get_id() const
// {
//     return _framebuffer_id;
// }

// }