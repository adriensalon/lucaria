#pragma once

#include <lucaria/core/owning.hpp>
#include <lucaria/core/sound_track.hpp>

namespace lucaria {
namespace detail {
    struct mixer_system;
}

/// @brief
struct speaker_component {
    speaker_component();
    speaker_component(const speaker_component& other) = delete;
    speaker_component& operator=(const speaker_component& other) = delete;
    speaker_component(speaker_component&& other) = default;
    speaker_component& operator=(speaker_component&& other) = default;
    ~speaker_component();

    speaker_component& use_sound(const sound_track_object sound_track);

    speaker_component& set_volume(const glm::float32 volume);
    speaker_component& set_play(const bool enable);
    speaker_component& set_loop(const bool enable);

    [[nodiscard]] std::optional<glm::uint> get_sample_rate() const;
    [[nodiscard]] std::optional<glm::uint> get_count() const;

private:
    detail::owning_flag _ownership = {};
    ALuint _handle;
    sound_track_object _sound = {};
    bool _is_playing = false;
    bool _want_playing = false;
    bool _is_looping = false;
    bool _want_looping = false;

    template <typename Archive>
    void save(Archive& archive) const
    {
        archive(cereal::make_nvp("sound", _sound));
        archive(cereal::make_nvp("want_playing", _want_playing));
        archive(cereal::make_nvp("want_looping", _want_looping));
    }

    template <typename Archive>
    void load(Archive& archive)
    {
        archive(cereal::make_nvp("sound", _sound));
        archive(cereal::make_nvp("want_playing", _want_playing));
        archive(cereal::make_nvp("want_looping", _want_looping));
        use_sound(_sound);
        set_loop(_want_looping);
        set_play(_want_playing);
    }

    friend struct detail::mixer_system;
    friend class cereal::access;
};

}
