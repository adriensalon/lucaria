#pragma once

#include <ozz/animation/runtime/track.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct system_motion;
    struct manager_assets;

    enum struct object_motion_track_origin {
        path
    };

    struct object_motion_track {
        LUCARIA_DELETE_DEFAULT(object_motion_track)
        object_motion_track(const object_motion_track& other) = delete;
        object_motion_track& operator=(const object_motion_track& other) = delete;
        object_motion_track(object_motion_track&& other) = default;
        object_motion_track& operator=(object_motion_track&& other) = default;

        object_motion_track(const std::vector<char>& bytes);
        object_motion_track(ozz::animation::Float3Track&& translation_track, ozz::animation::QuaternionTrack&& rotation_track);
        [[nodiscard]] float32x3 get_total_translation() const;

        object_motion_track_origin origin;
        ozz::animation::Float3Track translation_track;
        ozz::animation::QuaternionTrack rotation_track;
    };

    [[nodiscard]] assets_cell<object_motion_track>& fetch(
        manager_assets& objects,
        assets_buffer<object_motion_track>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_motion_track_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    using recipe_object_motion_track = std::variant<recipe_object_motion_track_path>;

    [[nodiscard]] recipe_object_motion_track make_recipe(const assets_cell<object_motion_track>& cache);
	[[nodiscard]] assets_cell<object_motion_track>* apply_recipe(manager_assets& objects, assets_buffer<object_motion_track>& cached, recipe_object_motion_track& recipe);

}
}
