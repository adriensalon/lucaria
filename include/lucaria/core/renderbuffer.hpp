#pragma once

#include <lucaria/core/texture.hpp>

namespace lucaria {

struct renderbuffer_ref {
    renderbuffer_ref() = delete;
    renderbuffer_ref(const renderbuffer_ref& other) = delete;
    renderbuffer_ref& operator=(const renderbuffer_ref& other) = delete;
    renderbuffer_ref(renderbuffer_ref&& other);
    renderbuffer_ref& operator=(renderbuffer_ref&& other);
    ~renderbuffer_ref();

    renderbuffer_ref(const glm::uint width, const glm::uint height, const glm::uint internal_format, const glm::uint samples = 1);
    glm::uint get_width() const;
    glm::uint get_height() const;
    glm::uint get_internal_format() const;
    glm::uint get_samples() const;
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _width;
    glm::uint _height;
    glm::uint _internal_format;
    glm::uint _samples;
    glm::uint _renderbuffer_id;
};

}
