#pragma once

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/sound.hpp>

namespace lucaria {

/// @brief
struct speaker_component {
    speaker_component();
    speaker_component(const speaker_component& other) = delete;
    speaker_component& operator=(const speaker_component& other) = delete;
    speaker_component(speaker_component&& other);
    speaker_component& operator=(speaker_component&& other);
    ~speaker_component();
    
    speaker_component& use_sound(sound_track& from);
    speaker_component& use_sound(fetched<sound_track>& from);

    speaker_component& set_volume(const glm::float32 volume);
    speaker_component& set_play(const bool enable);
    speaker_component& set_loop(const bool enable);

private:
    bool _is_owning = false;
    glm::uint _handle;
    _detail::fetched_container<sound_track> _sound = {};
    bool _is_playing = false;
    bool _want_playing = false;
    bool _is_looping = false;
    bool _want_looping = false;
    friend struct mixer_system;
};

}
