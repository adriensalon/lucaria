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
    
    static void step_simulation();
    static void compute_wall_slide();
    static void compute_ground_snap();
    static void compute_dynamics();
    static void compute_layers();
    static void collect_debug_guizmos();
};