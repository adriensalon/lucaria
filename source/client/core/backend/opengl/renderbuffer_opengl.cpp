#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {
namespace detail {

    renderbuffer_implementation::renderbuffer_implementation(renderbuffer_implementation&& other)
    {
        implementation_opengl.is_owning = false;
        *this = std::move(other);
    }

    renderbuffer_implementation& renderbuffer_implementation::operator=(renderbuffer_implementation&& other)
    {
        if (implementation_opengl.is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }

        implementation_opengl.is_owning = other.implementation_opengl.is_owning;
        implementation_opengl.id = other.implementation_opengl.id;
        implementation_opengl.internal_format = other.implementation_opengl.internal_format;
        sampling_count = other.sampling_count;
        size = other.size;

        other.implementation_opengl.is_owning = false;
        return *this;
    }

    renderbuffer_implementation::~renderbuffer_implementation()
    {
        if (implementation_opengl.is_owning) {
            glDeleteRenderbuffers(1, &implementation_opengl.id);
        }
    }

    renderbuffer_implementation::renderbuffer_implementation(const glm::uvec2 size, const glm::uint internal_format, const glm::uint samples)
        : size(size)
    {
        static GLint _max_samples = 1;
// #if defined(GL_MAX_SAMPLES)
        glGetIntegerv(GL_MAX_SAMPLES, &_max_samples);
// #endif
        sampling_count = static_cast<glm::uint>(std::clamp<int>(samples, 1, _max_samples));
        implementation_opengl.internal_format = internal_format;
        glGenRenderbuffers(1, &implementation_opengl.id);
        glBindRenderbuffer(GL_RENDERBUFFER, implementation_opengl.id);
// #if defined(GL_RENDERBUFFER_SAMPLES)
        if (sampling_count > 1) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(sampling_count), implementation_opengl.internal_format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        } else
// #endif
        {
            glRenderbufferStorage(GL_RENDERBUFFER, implementation_opengl.internal_format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        }

        implementation_opengl.is_owning = true;
    }

    void renderbuffer_implementation::resize(const glm::uvec2 new_size)
    {
        if (size == new_size) {
            return;
        }

        size = new_size;
        glBindRenderbuffer(GL_RENDERBUFFER, implementation_opengl.id);
        glRenderbufferStorage(GL_RENDERBUFFER, implementation_opengl.internal_format, size.x, size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

}
}