#pragma once

#include <lucaria/bin/data_audio.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct object_sound_track;
    struct manager_assets;

    enum struct object_audio_origin {
        path,
        data
    };

    struct object_audio {
        LUCARIA_DELETE_DEFAULT(object_audio)
        object_audio(const object_audio& other) = delete;
        object_audio& operator=(const object_audio& other) = delete;
        object_audio(object_audio&& other) = default;
        object_audio& operator=(object_audio&& other) = default;

        object_audio(const std::vector<char>& bytes);
        object_audio(data_audio&& data);
        // object_audio(const object_sound_track& sound_track); // NOT IMPLEMENTED YET

        object_audio_origin origin;
		std::optional<std::filesystem::path> origin_path;
		
        data_audio data;
    };

    [[nodiscard]] assets_cell<object_audio>& fetch(
        manager_assets& objects,
        assets_buffer<object_audio>& cached_vector,
        const std::filesystem::path& path);

    // recipes

    struct recipe_object_audio_path {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct recipe_object_audio_data {
        data_audio data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using recipe_object_audio = std::variant<recipe_object_audio_path, recipe_object_audio_data>;

    [[nodiscard]] recipe_object_audio make_recipe(const assets_cell<object_audio>& cache);
	[[nodiscard]] assets_cell<object_audio>* apply_recipe(manager_assets& objects, assets_buffer<object_audio>& cached, recipe_object_audio& recipe);
}
}
