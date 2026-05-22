#pragma once

#include <AL/al.h>

#include <lucaria/core/audio.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/owning.hpp>

namespace lucaria {
namespace detail {

    enum struct sound_track_origin {
        path,
        data
    };

    struct sound_track_implementation {
        LUCARIA_DELETE_DEFAULT(sound_track_implementation)
        sound_track_implementation(const sound_track_implementation& other) = delete;
        sound_track_implementation& operator=(const sound_track_implementation& other) = delete;
        sound_track_implementation(sound_track_implementation&& other) = default;
        sound_track_implementation& operator=(sound_track_implementation&& other) = default;
        ~sound_track_implementation();

        sound_track_implementation(const audio_implementation& from);

        sound_track_origin origin;
        owning_flag ownership = {};
        ALuint id;
        uint32 sample_rate;
        uint32 samples_count;
    };

    struct sound_track_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    struct sound_track_data_recipe {
        audio_data data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    using sound_track_recipe = std::variant<sound_track_path_recipe, sound_track_data_recipe>;

	[[nodiscard]] sound_track_recipe make_recipe(const implementation_container<sound_track_implementation>& container);
}

struct sound_track_object {
    sound_track_object() = default;
    sound_track_object(const sound_track_object& other) = default;
    sound_track_object& operator=(const sound_track_object& other) = default;
    sound_track_object(sound_track_object&& other) = default;
    sound_track_object& operator=(sound_track_object&& other) = default;

    static sound_track_object fetch(const std::filesystem::path& path);

    [[nodiscard]] bool has_value() const;

    [[nodiscard]] explicit operator bool() const;

private:
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::sound_track_implementation>* _manager = nullptr;
    detail::implementation_container<detail::sound_track_implementation>* _resource = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

	friend struct speaker_component;
	friend class cereal::access;
};

}
