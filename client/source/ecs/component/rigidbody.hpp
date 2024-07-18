#pragma once

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <core/mesh.hpp>

enum struct rigidbody_kind {
    dynamic,
    kinematic
};

struct kinematic_collision {

};

struct rigidbody_component {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other);
    rigidbody_component& operator=(rigidbody_component&& other);
    ~rigidbody_component();

    rigidbody_component& box(const glm::vec3& half_extents);
    rigidbody_component& capsule(const glm::float32 radius, const glm::float32 height);    
    rigidbody_component& snap_grounds(const bool enabled = true);
    rigidbody_component& glide_walls(const bool enabled = true);
    rigidbody_component& collide_layer(const glm::uint layer, const bool enabled = true);
    
    const std::vector<kinematic_collision>& get_kinematic_game_collisions(const glm::uint layer);


private:
    bool _is_instanced = false;
    btCollisionShape* _shape = nullptr;
    // btDefaultMotionState* _state = nullptr;
    // btRigidBody* _rigidbody = nullptr;
    btPairCachingGhostObject* _ghost = nullptr;
    friend struct dynamics_system;
};