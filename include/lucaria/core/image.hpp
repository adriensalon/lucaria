#pragma once

#include <array>
#include <optional>

#include <lucaria/common/image_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/platform.hpp>

namespace lucaria {

/// @brief Represents image data and metadata on the CPU. Exposes a loading interface
/// to image_data .bin assets. Images can be modified at runtime on the CPU for dynamic
/// or procedural workflows to update GPU textures when changes are needed
struct image {
    LUCARIA_DELETE_DEFAULT(image)
    image(const image& other) = delete;
    image& operator=(const image& other) = delete;
    image(image&& other) noexcept = default;
    image& operator=(image&& other) noexcept = default;

    /// @brief Creates an image from data
    /// @param data data to be moved from
    image(image_data&& data);

    /// @brief Creates an image from bytes synchronously
    /// @param image_bytes bytes to load from
    image(const std::vector<char>& data_bytes);

    /// @brief Loads an image from a file synchronously and lets the runtime
    /// choose the best format it can use without downloading the others
    /// @param raw_path path to load default uncompressed version from
    /// @param etc2_path path to load ETC2 compressed version from
    /// @param s3tc_path path to load S3TC compressed version from
    image(
        const std::filesystem::path& data_path,
        const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
        const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);

    image_data data;
};

/// @brief Loads an image from a file asynchronously and lets the runtime
/// choose the best format it can use without downloading the others
/// @param data_path path to load uncompressed version from
/// @param etc2_path path to load ETC2 compressed version from
/// @param s3tc_path path to load S3TC compressed version from
[[nodiscard]] fetched<image> fetch_image(
    const std::filesystem::path& data_path,
    const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
    const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);

}
