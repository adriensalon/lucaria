#pragma once

#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct system_motion;
    struct manager_assets;

    enum struct object_skeleton_origin {
        path
    };

    struct object_skeleton {
        object_skeleton() = default;
        object_skeleton(const object_skeleton& other) = delete;
        object_skeleton& operator=(const object_skeleton& other) = delete;
        object_skeleton(object_skeleton&& other) = default;
        object_skeleton& operator=(object_skeleton&& other) = default;

        object_skeleton(const std::vector<char>& bytes);

        object_skeleton_origin origin;
		std::filesystem::path origin_path;		
        ozz::animation::Skeleton skeleton;

        template <typename ContextType>
        void save(ContextType& context) const
        {
            context(cereal::make_nvp("origin", origin));
            if (origin == object_skeleton_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
            }
        }

        template <typename ContextType>
        void load(ContextType& context)
        {
            context(cereal::make_nvp("origin", origin));
            if (origin == object_skeleton_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_skeleton(bytes);
                    origin_path = _path;
                });
            }
        }

    };

    [[nodiscard]] assets_cell<object_skeleton>& fetch(
		manager_assets& objects,
        assets_buffer<object_skeleton>& cached_vector,
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

    [[nodiscard]] recipe_object_skeleton make_recipe(const assets_cell<object_skeleton>& cache);
	[[nodiscard]] assets_cell<object_skeleton>* apply_recipe(manager_assets& objects, assets_buffer<object_skeleton>& cached, recipe_object_skeleton& recipe);

}
}
