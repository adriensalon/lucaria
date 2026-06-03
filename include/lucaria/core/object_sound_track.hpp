#pragma once

#include <AL/al.h>

#include <lucaria/core/object_audio.hpp>
#include <lucaria/core/utils_owning.hpp>

namespace lucaria {
namespace detail {
	
    struct manager_assets;

    enum struct object_sound_track_origin {
        path,
        data
    };

    struct object_sound_track {
        LUCARIA_DELETE_DEFAULT(object_sound_track)
        object_sound_track(const object_sound_track& other) = delete;
        object_sound_track& operator=(const object_sound_track& other) = delete;
        object_sound_track(object_sound_track&& other) = default;
        object_sound_track& operator=(object_sound_track&& other) = default;
        ~object_sound_track();

        object_sound_track(const object_audio& from);

        object_sound_track_origin origin;
		std::optional<std::filesystem::path> origin_path;
		
        flag_owning ownership = {};
        ALuint id;
        uint32 sample_rate;
        uint32 samples_count;
    };

    [[nodiscard]] assets_cell<object_sound_track>& fetch(
		manager_assets& objects,
        assets_buffer<object_sound_track>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_sound_track_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct recipe_object_sound_track_data {
        data_audio data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_sound_track = std::variant<recipe_object_sound_track_path, recipe_object_sound_track_data>;

    [[nodiscard]] recipe_object_sound_track make_recipe(const assets_cell<object_sound_track>& cache);
	[[nodiscard]] assets_cell<object_sound_track>* apply_recipe(manager_assets& objects, assets_buffer<object_sound_track>& cached, recipe_object_sound_track& recipe);

}
}
