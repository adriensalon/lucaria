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
        object_sound_track() = default;
        object_sound_track(const object_sound_track& other) = delete;
        object_sound_track& operator=(const object_sound_track& other) = delete;
        object_sound_track(object_sound_track&& other) = default;
        object_sound_track& operator=(object_sound_track&& other) = default;
        ~object_sound_track();

        object_sound_track(const object_audio& from);

        object_sound_track_origin origin;
		std::filesystem::path origin_path;		
        flag_owning ownership = {};
        ALuint id;
        uint32 sample_rate;
        uint32 samples_count;

        template <typename ContextType>
        void save(ContextType& context) const
        {
            context(cereal::make_nvp("origin", origin));
            if (origin == object_sound_track_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
            }
        }

        template <typename ContextType>
        void load(ContextType& context)
        {
            context(cereal::make_nvp("origin", origin));
            if (origin == object_sound_track_origin::path) {
                context(cereal::make_nvp("origin_path", origin_path));
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    object_audio _audio(bytes);
                    *this = object_sound_track(_audio);
                    origin_path = _path;
                });
            }
        }

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
