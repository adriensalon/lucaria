#pragma once

#include <array>
#include <optional>

#include <lucaria/common/image_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents a runtime image on the host
struct image {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(image)
    image(const image& other) = delete;
    image& operator=(const image& other) = delete;
    image(image&& other) noexcept = default;
    image& operator=(image&& other) noexcept = default;

    /// @brief Creates an empty image from dimensions
    /// @param data data to be moved from
    image(const glm::uint channels, const glm::uint width, const glm::uint height);

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

namespace detail {

    [[nodiscard]] const std::filesystem::path& resolve_image_path(
        const std::filesystem::path& data_path,
        const std::optional<std::filesystem::path>& etc2_path,
        const std::optional<std::filesystem::path>& s3tc_path);

    [[nodiscard]] std::vector<std::filesystem::path> resolve_image_paths(
        const std::array<std::filesystem::path, 6>& data_paths,
        const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths,
        const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths);
}

}
