#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;

    enum struct object_animation_origin {
        path
    };

    struct object_animation {
        object_animation() = default;
        object_animation(const object_animation& other) = delete;
        object_animation& operator=(const object_animation& other) = delete;
        object_animation(object_animation&& other) = default;
        object_animation& operator=(object_animation&& other) = default;

        object_animation(const std::vector<char>& bytes);

        object_animation_origin origin;
        std::filesystem::path origin_path;
        ozz::animation::Animation animation;

        template <typename Archive>
        void save(Archive& archive)
        {
            archive(cereal::make_nvp("origin", origin));
            archive(cereal::make_nvp("origin_path", origin_path));
        }

        template <typename Archive>
        void load(Archive& archive)
        {
            archive(cereal::make_nvp("origin", origin));
            archive(cereal::make_nvp("origin_path", origin_path));
            const std::filesystem::path _path = origin_path;
            archive.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                *this = object_animation(bytes);
                origin_path = _path;
            });
        }
    };

    [[nodiscard]] assets_cell<object_animation>& fetch(
        manager_assets& objects,
        assets_buffer<object_animation>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_animation_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    using recipe_object_animation = std::variant<recipe_object_animation_path>;

    [[nodiscard]] recipe_object_animation make_recipe(const assets_cell<object_animation>& cache);
    [[nodiscard]] assets_cell<object_animation>* apply_recipe(manager_assets& objects, assets_buffer<object_animation>& cached, recipe_object_animation& recipe);

}
}
