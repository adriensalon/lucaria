#pragma once

#include <lucaria/core/image.hpp>

namespace lucaria {

/// @brief Represents a runtime texture on the device
struct texture {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(texture)
    texture(const texture& other) = delete;
    texture& operator=(const texture& other) = delete;
    texture(texture&& other);
    texture& operator=(texture&& other);
    ~texture();

    /// @brief Creates a texture from image data
    /// @param from the image data to create from
    texture(const image& from);

    /// @brief Creates an empty texture from pixels size
    /// @param size the size to create from
    texture(const glm::uvec2 size);

    /// @brief Sets a new size for the texture and reset pixels
    /// @param size the size to update to
    void resize(const glm::uvec2 size);

    /// @brief Updates the pixels buffer from image data
    /// @param from the image data to update from
    /// @param size the selected count to update
    /// @param offset the selected offset to update from
    void update_pixels(const image& from, const glm::uvec2 size, const glm::uvec2 offset = { 0, 0 });

    /// @brief Returns the texture pixels count
    /// @return the pixels count along U and V
    [[nodiscard]] glm::uvec2 get_size() const;
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;

private:
    bool _is_owning;
    glm::uvec2 _size;
    glm::uint _handle;
};

/// @brief Loads an image from a file asynchronously and uploads directly to the device,
/// lets the runtime choose the best format it can use without downloading the others
/// @param data_path path to load uncompressed image version from
/// @param etc2_path path to load ETC2 compressed image version from
/// @param s3tc_path path to load S3TC compressed image version from
[[nodiscard]] fetched<texture> fetch_texture(const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
    const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);

}
