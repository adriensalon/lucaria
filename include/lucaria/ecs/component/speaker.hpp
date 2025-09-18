#pragma once

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/sound.hpp>
#include <lucaria/core/world.hpp>

struct sound_controller {

    // do the same as animation_controller

    bool is_playing = true;
    bool is_looping = false;
};

struct speaker_component {
    speaker_component() = default;
    speaker_component(const speaker_component& other) = delete;
    speaker_component& operator=(const speaker_component& other) = delete;
    speaker_component(speaker_component&& other) = default; // todo
    speaker_component& operator=(speaker_component&& other) = default; // todo
 // todo delete
 
    speaker_component& sounds(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<sound_ref>>>& fetched_sounds);

    sound_controller& get_controller(const glm::uint name);

private:
    bool _is_instanced = false;
    std::unordered_map<glm::uint, sound_controller> _controllers = {};
    std::unordered_map<glm::uint, fetch_container<sound_ref>> _sounds = {};
    std::unordered_map<glm::uint, glm::uint> _source_ids = {};
    friend struct mixer_system;
};