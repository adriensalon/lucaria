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

    /// @brief
    /// @param from
    texture(const image& from);

    /// @brief
    /// @param size
    texture(const glm::uvec2 size);

    // set parameters

    // generate mipmaps

    /// @brief
    /// @param offset
    /// @param from
    void update_pixels(const image& from, const glm::uvec2 size, const glm::uvec2 offset = { 0, 0 });

    [[nodiscard]] glm::uvec2 get_size() const;
    [[nodiscard]] glm::uint get_handle() const;

private:
    bool _is_owning;
    glm::uvec2 _size;
    glm::uint _handle;
};

/// @brief Loads an image from a file asynchronously and uploads directly to the device,
/// lets the runtime choose the best format it can use without downloading the others
/// @param image_data_path path to load uncompressed image version from
/// @param image_etc2_path path to load ETC2 compressed image version from
/// @param image_s3tc_path path to load S3TC compressed image version from
[[nodiscard]] fetched<texture> fetch_texture(const std::filesystem::path& image_data_path,
    const std::optional<std::filesystem::path>& image_etc2_path = std::nullopt,
    const std::optional<std::filesystem::path>& image_s3tc_path = std::nullopt);

}
