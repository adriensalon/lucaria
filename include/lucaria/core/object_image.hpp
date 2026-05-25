#pragma once

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct object_cubemap;
    struct object_texture;
    struct manager_object;

    enum struct object_image_origin {
        path,
        data
    };

    struct object_image {
        LUCARIA_DELETE_DEFAULT(object_image)
        object_image(const object_image& other) = delete;
        object_image& operator=(const object_image& other) = delete;
        object_image(object_image&& other) noexcept = default;
        object_image& operator=(object_image&& other) noexcept = default;

        object_image(const std::vector<char>& bytes);
        object_image(data_image&& data);
        // object_image(const object_texture& texture);
        // object_image(const object_cubemap& cubemap, const uint32 face_index);

        object_image_origin origin;
        data_image data;
    };

    [[nodiscard]] std::filesystem::path resolve_profile(
        manager_object& objects,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile = std::nullopt);

    [[nodiscard]] std::array<std::filesystem::path, 6> resolve_profile(
        manager_object& objects,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile = std::nullopt);

    [[nodiscard]] container_cache<object_image>& fetch(
        manager_object& objects,
        container_cache_vector<object_image>& cache_vector,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile = std::nullopt);

    // recipes

    struct recipe_object_image_path {
        std::filesystem::path path;
        data_image_profile profile;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("profile", profile));
        }
    };

    struct recipe_object_image_data {
        data_image data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_image = std::variant<recipe_object_image_path, recipe_object_image_data>;

    [[nodiscard]] recipe_object_image make_recipe(const container_cache<object_image>& cache);
}
}
