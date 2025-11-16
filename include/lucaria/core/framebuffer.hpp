#pragma once

#include <lucaria/core/renderbuffer.hpp>

namespace lucaria {

/// @brief Represents a framebuffer on the device
struct framebuffer {
    framebuffer() = delete;
    framebuffer(const framebuffer& other) = delete;
    framebuffer& operator=(const framebuffer& other) = delete;
    framebuffer(framebuffer&& other);
    framebuffer& operator=(framebuffer&& other);
    ~framebuffer();

    /// @brief Creates a framebuffer from size
    /// @param size size to create from
    framebuffer(const glm::uvec2& size);

    /// @brief Uses this framebuffer for draw calls
    void use();

    /// @brief Binds a texture object for the color attachment of this framebuffer
    /// @param color the texture object to bind
    void bind_color(texture& color);

    // TODO
    void bind_color(deprecated_renderbuffer& color);

    /// @brief Binds a texture object for the depth attachment of this framebuffer
    /// @param color the texture object to bind
    void bind_depth(texture& depth);

    // TODO
    void bind_depth(deprecated_renderbuffer& depth);
    
    /// @brief Returns the framebuffer pixels count
    /// @return the pixels count along U and V
    [[nodiscard]] glm::uvec2 get_size() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;

    /// @brief Uses the default framebuffer for draw calls
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
