#pragma once

#include <lucaria/core/image.hpp>

namespace lucaria {

/// @brief Represents a runtime cubemap texture on the device
struct cubemap {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(cubemap)
    cubemap(const cubemap& other) = delete;
    cubemap& operator=(const cubemap& other) = delete;
    cubemap(cubemap&& other);
    cubemap& operator=(cubemap&& other);
    ~cubemap();

    /// @brief Creates a cubemap from images synchronously
    /// @param images the images to create from
    cubemap(const std::array<image, 6>& images);

    // set parameters

    // generate mipmaps

    // update pixels

    [[nodiscard]] glm::uint get_handle() const;

private:
    bool _is_owning;
    glm::uint _handle;
};

/// @brief Loads a cubemap from files asynchronously and uploads directly to the device,
/// lets the runtime choose the best format it can use without downloading the others
/// @param image_data_paths paths to load uncompressed image versions from
/// @param image_etc2_paths paths to load ETC2 compressed image versions from
/// @param image_s3tc_paths paths to load S3TC compressed image versions from
[[nodiscard]] fetched<cubemap> fetch_cubemap(
    const std::array<std::filesystem::path, 6>& image_data_paths,
    const std::optional<std::array<std::filesystem::path, 6>>& image_etc2_paths = std::nullopt,
    const std::optional<std::array<std::filesystem::path, 6>>& image_s3tc_paths = std::nullopt);

}
