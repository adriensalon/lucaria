#pragma once

#include <glm/glm.hpp>

struct dynamics_system {
    dynamics_system() = delete;
    dynamics_system(const dynamics_system& other) = delete;
    dynamics_system& operator=(const dynamics_system& other) = delete;
    dynamics_system(dynamics_system&& other) = delete;
    dynamics_system& operator=(dynamics_system&& other) = delete;

    static void gravity(const glm::vec3& newtons);

    static void update();


    // static void use_on_collision_callback(const std::function<void()>& callback);
    // static void use_on_collision_callback(const std::function<void()>& callback);

    static void prevent_kinematic_wall_collisions(); // transforms, rigidbodies \\ colliders (from bullet)
    static void snap_kinematic_grounds(); // transforms, rigidbodies \\ colliders (from bullet)
};