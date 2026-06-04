#pragma once

#include <lucaria/bin/data_image.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct object_cubemap;
    struct object_texture;
    struct manager_assets;

    enum struct object_image_origin {
        path,
        data
    };

    struct object_image {
        object_image() = default;
        object_image(const object_image& other) = delete;
        object_image& operator=(const object_image& other) = delete;
        object_image(object_image&& other) noexcept = default;
        object_image& operator=(object_image&& other) noexcept = default;

        object_image(const std::vector<char>& bytes);
        object_image(data_image&& data);
        // object_image(const object_texture& texture);
        // object_image(const object_cubemap& cubemap, const uint32 face_index);

        object_image_origin origin;
		std::filesystem::path origin_path;		
		data_image_profile profile;
        data_image data;

        template <typename ContextType>
        void save(ContextType& context) const
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("profile", profile));
            if (origin == object_image_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
            }
            if (origin == object_image_origin::data) {
                context(cereal::make_nvp("origin_data", data));
            }
        }

        template <typename ContextType>
        void load(ContextType& context)
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("profile", profile));
            if (origin == object_image_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
                const std::filesystem::path _path = origin_path;
                const data_image_profile _profile = profile;
                const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, _profile);
                context.fetch(_resolved_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_image(bytes);
                    origin_path = _path;
                });
            }
            if (origin == object_image_origin::data) {
                context(cereal::make_nvp("origin_data", data));
            }
        }

    };

    [[nodiscard]] std::filesystem::path resolve_profile(
        manager_assets& objects,
        const std::filesystem::path& path,
        const std::optional<data_image_profile> profile = std::nullopt);

    [[nodiscard]] std::array<std::filesystem::path, 6> resolve_profile(
        manager_assets& objects,
        const std::array<std::filesystem::path, 6>& paths,
        const std::optional<data_image_profile> profile = std::nullopt);

    [[nodiscard]] assets_cell<object_image>& fetch(
        manager_assets& objects,
        assets_buffer<object_image>& cache_vector,
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

    [[nodiscard]] recipe_object_image make_recipe(const assets_cell<object_image>& cache);
	[[nodiscard]] assets_cell<object_image>* apply_recipe(manager_assets& objects, assets_buffer<object_image>& cached, recipe_object_image& recipe);

}
}
