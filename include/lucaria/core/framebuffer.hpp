#pragma once

#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {

struct framebuffer {
    framebuffer() = delete;
    framebuffer(const framebuffer& other) = delete;
    framebuffer& operator=(const framebuffer& other) = delete;
    framebuffer(framebuffer&& other);
    framebuffer& operator=(framebuffer&& other);
    ~framebuffer();

    framebuffer(const glm::uvec2 size);
    void bind_color(const texture& color);
    void bind_color(const renderbuffer_ref& color);
    void bind_depth(const texture& depth);
    void bind_depth(const renderbuffer_ref& depth);
    
    glm::uvec2 size;
    glm::uint handle;

private:
    bool _is_owning;
    std::optional<glm::uint> _texture_color_id;
    std::optional<glm::uint> _texture_depth_id;
    std::optional<glm::uint> _renderbuffer_color_id;
    std::optional<glm::uint> _renderbuffer_depth_id;
};

}
