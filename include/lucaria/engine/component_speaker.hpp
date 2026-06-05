#pragma once

#include <lucaria/engine/asset_sound_track.hpp>

namespace lucaria {
namespace detail {
    struct system_mixer;
}

/// @brief
struct component_speaker_spatial {
    component_speaker_spatial();
    component_speaker_spatial(const component_speaker_spatial& other) = delete;
    component_speaker_spatial& operator=(const component_speaker_spatial& other) = delete;
    component_speaker_spatial(component_speaker_spatial&& other) = default;
    component_speaker_spatial& operator=(component_speaker_spatial&& other) = default;
    ~component_speaker_spatial();

    component_speaker_spatial& use_sound(const handle_sound_track sound_track);

    component_speaker_spatial& set_volume(const glm::float32 volume);
    component_speaker_spatial& set_play(const bool enable);
    component_speaker_spatial& set_loop(const bool enable);

    [[nodiscard]] std::optional<glm::uint> get_sample_rate() const;
    [[nodiscard]] std::optional<glm::uint> get_count() const;

private:
    detail::flag_owning _ownership = {};
    ALuint _handle;
    handle_sound_track _sound = {};
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

    friend struct detail::system_mixer;
    friend class cereal::access;
};

}
