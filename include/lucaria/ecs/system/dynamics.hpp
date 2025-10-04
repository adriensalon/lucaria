#pragma once

#include <glm/glm.hpp>

namespace lucaria {
namespace ecs {

    namespace dynamics {

        void set_gravity(const glm::vec3& newtons);
        void set_snap_ground_distance(const glm::float32 meters);
    }
}

namespace detail {

    struct dynamics_system {
        dynamics_system() = delete;
        dynamics_system(const dynamics_system& other) = delete;
        dynamics_system& operator=(const dynamics_system& other) = delete;
        dynamics_system(dynamics_system&& other) = delete;
        dynamics_system& operator=(dynamics_system&& other) = delete;

        static void step_characters();
        static void step_simulation();
        static void compute_collisions();
        static void collect_debug_guizmos();
    };
}
}
