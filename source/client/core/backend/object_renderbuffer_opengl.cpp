#include <lucaria/core/rendering_renderbuffer.hpp>

namespace lucaria {
namespace detail {

    object_renderbuffer::~object_renderbuffer()
    {
        if (ownership.owns()) {
            glDeleteRenderbuffers(1, &id);
        }
    }

    object_renderbuffer::object_renderbuffer(const glm::uvec2 size, const uint32 format, const uint32 samples)
        : size(size)
		, internal_format(format)
    {
        static GLint _max_samples = 1;
        glGetIntegerv(GL_MAX_SAMPLES, &_max_samples);
        sampling_count = static_cast<uint32>(std::clamp<int>(samples, 1, _max_samples));
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        if (sampling_count > 1) {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, static_cast<GLsizei>(sampling_count), internal_format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        } else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, internal_format, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        }

        ownership.emplace();
    }

    void object_renderbuffer::resize(const glm::uvec2 new_size)
    {
        if (size == new_size) {
            return;
        }

        size = new_size;
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, internal_format, size.x, size.y);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

}
}