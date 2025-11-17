#pragma once

#include <lucaria/core/texture.hpp>

namespace lucaria {

/// @brief Represents a runtime renderbuffer on the device
struct renderbuffer {
    renderbuffer() = delete;
    renderbuffer(const renderbuffer& other) = delete;
    renderbuffer& operator=(const renderbuffer& other) = delete;
    renderbuffer(renderbuffer&& other);
    renderbuffer& operator=(renderbuffer&& other);
    ~renderbuffer();

    /// @brief Creates a framebuffer from size, internal format and requested samples
    /// @param size size to create from
    /// @param internal_format GL internal texture format to use
    /// @param samples requested sample count
    renderbuffer(const glm::uvec2 size, const glm::uint internal_format, const glm::uint samples = 1);
    
    /// @brief Sets a new size for the renderbuffer
    /// @param size the size to update to
    void resize(const glm::uvec2 size);

    /// @brief Returns the renderbuffer pixels count
    /// @return the pixels count along U and V
    [[nodiscard]] glm::uvec2 get_size() const;
    

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;
    

    /// @brief Returns the GL internal texture format
    /// @return the internal texture format
    [[nodiscard]] glm::uint get_internal_format() const;
    

    /// @brief Returns the availables samples used by this renderbuffer
    /// @return samples clamped by maximal samples for the implementation
    [[nodiscard]] glm::uint get_samples() const;

private:
    bool _is_owning;
    glm::uvec2 _size;
    glm::uint _handle;
    glm::uint _internal_format;
    glm::uint _samples;
};

}
