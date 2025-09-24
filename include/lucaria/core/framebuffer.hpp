#pragma once

#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {

struct framebuffer_ref {
    framebuffer_ref() = delete;
    framebuffer_ref(const framebuffer_ref& other) = delete;
    framebuffer_ref& operator=(const framebuffer_ref& other) = delete;
    framebuffer_ref(framebuffer_ref&& other);
    framebuffer_ref& operator=(framebuffer_ref&& other);
    ~framebuffer_ref();

    framebuffer_ref(const glm::uint width, const glm::uint height);
    void color(const texture_ref& color_tex);
    void color(const renderbuffer_ref& color_rb);
    void depth(const texture_ref& depth_tex);
    void depth(const renderbuffer_ref& depth_rb);
    glm::uint get_width() const;
    glm::uint get_height() const;
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _width;
    glm::uint _height;
    glm::uint _framebuffer_id;
    std::optional<glm::uint> _texture_color_id;
    std::optional<glm::uint> _texture_depth_id;
    std::optional<glm::uint> _renderbuffer_color_id;
    std::optional<glm::uint> _renderbuffer_depth_id;
};

}
