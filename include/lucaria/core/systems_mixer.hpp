#pragma once

#include <lucaria/core/game_access.hpp>

namespace lucaria {

struct component_transform;

namespace detail {

	struct manager_scenes;

    struct system_mixer {
        system_mixer() = default;
        system_mixer(const system_mixer& other) = delete;
        system_mixer& operator=(const system_mixer& other) = delete;
        system_mixer(system_mixer&& other) = default;
        system_mixer& operator=(system_mixer&& other) = default;

        component_transform* listener_transform = nullptr;
        // handle_entity listener_entity = {};

        void update_apply_speaker_transforms(manager_scenes& scenes);
        void update_apply_listener_transform(manager_scenes& scenes);
    };

}

}
