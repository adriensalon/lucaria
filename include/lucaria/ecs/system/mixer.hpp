#pragma once

#include <lucaria/ecs/component/transform.hpp>

struct mixer_system {
    mixer_system() = delete;
    mixer_system(const mixer_system& other) = delete;
    mixer_system& operator=(const mixer_system& other) = delete;
    mixer_system(mixer_system&& other) = delete;
    mixer_system& operator=(mixer_system&& other) = delete;

    static void use_listener_transform(transform_component& transform);

    static void apply_speaker_transforms();
    static void apply_listener_transform();
};