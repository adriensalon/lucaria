#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

	struct manager_object;

    enum struct object_animation_origin {
        path
    };

    struct object_animation {
        LUCARIA_DELETE_DEFAULT(object_animation)
        object_animation(const object_animation& other) = delete;
        object_animation& operator=(const object_animation& other) = delete;
        object_animation(object_animation&& other) = default;
        object_animation& operator=(object_animation&& other) = default;

        object_animation(const std::vector<char>& bytes);
        object_animation(ozz::animation::Animation&& animation);

        object_animation_origin origin;
        ozz::animation::Animation animation;
    };

    [[nodiscard]] container_cache<object_animation>& fetch(
		manager_object& objects,
        container_cache_vector<object_animation>& cached_vector,
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

    [[nodiscard]] recipe_object_animation make_recipe(const container_cache<object_animation>& cache);
	[[nodiscard]] container_cache<object_animation>* apply_recipe(manager_object& objects, container_cache_vector<object_animation>& cached, recipe_object_animation& recipe);

}
}
