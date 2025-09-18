#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

struct dynamics_system {
    dynamics_system() = delete;
    dynamics_system(const dynamics_system& other) = delete;
    dynamics_system& operator=(const dynamics_system& other) = delete;
    dynamics_system(dynamics_system&& other) = delete;
    dynamics_system& operator=(dynamics_system&& other) = delete;

    static void use_gravity(const glm::vec3& newtons);
    static void use_snap_ground_distance(const glm::float32 meters);

    static void step_simulation();
    static void compute_kinematic_collisions();
    static void collect_debug_guizmos();
};