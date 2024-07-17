#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

struct rigidbody_component {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other);
    rigidbody_component& operator=(rigidbody_component&& other);
    ~rigidbody_component();

    rigidbody_component& box(const glm::vec3& half_extents);
    rigidbody_component& capsule(const glm::float32 radius, const glm::float32 height);

private:
    bool _is_instanced = false;
    btCollisionShape* _shape = nullptr;
    btDefaultMotionState* _state = nullptr;
    btRigidBody* _rigidbody = nullptr;
    friend struct dynamics_system;
};