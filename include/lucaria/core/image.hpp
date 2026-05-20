#pragma once

#include <array>
#include <optional>

#include <lucaria/bin/image_data.hpp>
#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>

namespace lucaria {
namespace detail {

    struct image_implementation {
        LUCARIA_DELETE_DEFAULT(image_implementation)
        image_implementation(const image_implementation& other) = delete;
        image_implementation& operator=(const image_implementation& other) = delete;
        image_implementation(image_implementation&& other) noexcept = default;
        image_implementation& operator=(image_implementation&& other) noexcept = default;

        image_implementation(const std::vector<char>& bytes);
        image_implementation(image_data&& data);

        image_data data;
    };

}

/// @brief Represents an image in CPU memory. Can be created from an image file or from an empty size.
struct image_object {
    image_object() = default;
    image_object(const image_object& other) = default;
    image_object& operator=(const image_object& other) = default;
    image_object(image_object&& other) = default;
    image_object& operator=(image_object&& other) = default;

    /// @brief Creates a new image with the specified size
    /// @param size	the size of the image to create
    /// @return a image object with an empty image of the specified size
    static image_object create(const glm::uvec2 size);

    /// @brief Loads an image from a file asynchronously and uploads directly to the device,
    /// lets the runtime choose the best format it can use without downloading the others
    /// @param path path to load uncompressed image version from
    /// @param etc2_path path to load ETC2 compressed image version from
    /// @param s3tc_path path to load S3TC compressed image version from
    static image_object fetch(
        const std::filesystem::path& path,
        const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
        const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);

    /// @brief Checks if the image is ready to be used
    /// @return true if the image is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

    /// @brief Resizes the image to the specified size
    /// @param size the new size of the image
    void resize(const glm::uvec2 size);

    /// @brief Updates the pixels of the image with the specified image data, size and offset
    /// @param from the image data to update from
    /// @param size the size of the image data
    /// @param offset the offset within the image to update
    // void update_pixels(const  from, const glm::uvec2 size, const glm::uvec2 offset = { 0, 0 });

    /// @brief Gets the size of the image
    /// @return the size of the image
    [[nodiscard]] glm::uvec2 get_size() const;

private:
    detail::resource_container<detail::image_implementation>* _resource = nullptr;
    explicit image_object(detail::resource_container<detail::image_implementation>* resource);
};

}
