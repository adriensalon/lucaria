#pragma once

#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct system_motion;
    struct manager_assets;

    enum struct object_skeleton_origin {
        path
    };

    struct object_skeleton {
        LUCARIA_DELETE_DEFAULT(object_skeleton)
        object_skeleton(const object_skeleton& other) = delete;
        object_skeleton& operator=(const object_skeleton& other) = delete;
        object_skeleton(object_skeleton&& other) = default;
        object_skeleton& operator=(object_skeleton&& other) = default;

        object_skeleton(const std::vector<char>& bytes);
        object_skeleton(ozz::animation::Skeleton&& skeleton);

        object_skeleton_origin origin;
        ozz::animation::Skeleton skeleton;
    };

    [[nodiscard]] container_cache<object_skeleton>& fetch(
		manager_assets& objects,
        container_cache_vector<object_skeleton>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_skeleton_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    using recipe_object_skeleton = std::variant<recipe_object_skeleton_path>;

    [[nodiscard]] recipe_object_skeleton make_recipe(const container_cache<object_skeleton>& cache);
	[[nodiscard]] container_cache<object_skeleton>* apply_recipe(manager_assets& objects, container_cache_vector<object_skeleton>& cached, recipe_object_skeleton& recipe);

}
}
