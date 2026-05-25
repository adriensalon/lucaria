#pragma once

#include <lucaria/bin/data_event_track.hpp>
#include <lucaria/core/utils_cache.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

	struct manager_object;

    enum struct object_event_track_origin {
        path,
		data
    };

    struct object_event_track {
        LUCARIA_DELETE_DEFAULT(object_event_track)
        object_event_track(const object_event_track& other) = delete;
        object_event_track& operator=(const object_event_track& other) = delete;
        object_event_track(object_event_track&& other) = default;
        object_event_track& operator=(object_event_track&& other) = default;

        object_event_track(const std::vector<char>& bytes);
        object_event_track(data_event_track&& data);

        object_event_track_origin origin;
        data_event_track data;
    };

    [[nodiscard]] container_cache<object_event_track>& fetch(
		manager_object& objects, 
        container_cache_vector<object_event_track>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_event_track_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct recipe_object_event_track_data {
        data_event_track data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_event_track = std::variant<recipe_object_event_track_path, recipe_object_event_track_data>;

    [[nodiscard]] recipe_object_event_track make_recipe(const container_cache<object_event_track>& cache);

}
}
