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
    void use();
    void bind_color(texture& color);
    void bind_color(renderbuffer_ref& color);
    void bind_depth(texture& depth);
    void bind_depth(renderbuffer_ref& depth);
    
    [[nodiscard]] glm::uvec2 get_size() const;
    [[nodiscard]] glm::uint get_handle() const;

    static void use_default();

private:
    bool _is_owning;
    glm::uvec2 _size;
    glm::uint _handle;
    std::optional<glm::uint> _texture_color_id;
    std::optional<glm::uint> _texture_depth_id;
    std::optional<glm::uint> _renderbuffer_color_id;
    std::optional<glm::uint> _renderbuffer_depth_id;
};

}
