#pragma once

#include <lucaria/ecs/component/transform.hpp>

namespace lucaria {
namespace ecs {

    namespace mixer {

        void use_listener_transform(transform_component& transform);
    }
}

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
