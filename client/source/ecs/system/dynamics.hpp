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
    
    static btDiscreteDynamicsWorld* get_dynamics_world();

    static void prevent_kinematic_wall_collisions();
    static void snap_kinematic_grounds();
};