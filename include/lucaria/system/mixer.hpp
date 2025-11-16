#pragma once

#include <lucaria/component/transform.hpp>

namespace lucaria {

/// @brief Uses a transform component as the audio listener
/// @param transform the transform component to use
void use_listener_transform(transform_component& transform);

namespace detail {

    struct mixer_system {
        mixer_system() = delete;
        mixer_system(const mixer_system& other) = delete;
        mixer_system& operator=(const mixer_system& other) = delete;
        mixer_system(mixer_system&& other) = delete;
        mixer_system& operator=(mixer_system&& other) = delete;

        static void apply_speaker_transforms();
        static void apply_listener_transform();
    };
}
}
