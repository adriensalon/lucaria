#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <core/layer.hpp>
#include <core/mesh.hpp>

enum struct rigidbody_kind {
    kinematic,
    dynamic
};

enum struct rigidbody_layer : short {
    layer_0 = bulletgroupID_collider_layer_0,
    layer_1 = bulletgroupID_collider_layer_1,
    layer_2 = bulletgroupID_collider_layer_2,
    layer_3 = bulletgroupID_collider_layer_3,
    layer_4 = bulletgroupID_collider_layer_4,
    layer_5 = bulletgroupID_collider_layer_5,
    layer_6 = bulletgroupID_collider_layer_6,
    layer_7 = bulletgroupID_collider_layer_7,
};

template <rigidbody_kind kind_t>
struct rigidbody_component;

struct kinematic_collision {
    // todo 
    glm::vec3 impact_position;
    glm::vec3 impact_normal;
};

template <>
struct rigidbody_component<rigidbody_kind::kinematic> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other);
    rigidbody_component& operator=(rigidbody_component&& other);
    ~rigidbody_component();

    rigidbody_component& box(const glm::vec3& half_extents);
    rigidbody_component& capsule(const float radius, const float height);    
    rigidbody_component& snap_ground(const bool enabled = true);
    rigidbody_component& glide_wall(const bool enabled = true);
    rigidbody_component& collide_layer(const rigidbody_layer layer, const bool enabled = true);
    rigidbody_component& fill_kinematic_collisions(const rigidbody_layer layer, std::vector<kinematic_collision>& collisions);

private:
    bool _is_instanced = false;
    btCollisionShape* _shape = nullptr;
    btPairCachingGhostObject* _ghost = nullptr;
    short _group = bulletgroupID_kinematic_rigidbody;
    short _mask = 0;
    std::unordered_map<rigidbody_layer, std::vector<kinematic_collision>> _collisions = {};
    friend struct dynamics_system;
};

template <>
struct rigidbody_component<rigidbody_kind::dynamic> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other);
    rigidbody_component& operator=(rigidbody_component&& other);
    ~rigidbody_component();

    rigidbody_component& mass(const float kilograms);
    rigidbody_component& box(const glm::vec3& half_extents);
    rigidbody_component& capsule(const float radius, const float height);
    rigidbody_component& collide_dynamics(const bool enabled = true);
    
private:
    bool _is_instanced = false;
    btCollisionShape* _shape = nullptr;
    btDefaultMotionState* _state = nullptr;
    btRigidBody* _rigidbody = nullptr;
    float _mass = 0.f;
    short _group = bulletgroupID_dynamic_rigidbody;
    short _mask = bulletgroupID_collider_ground | bulletgroupID_collider_wall;
    friend struct dynamics_system;
};

using kinematic_rigidbody_component = rigidbody_component<rigidbody_kind::kinematic>;
using dynamic_rigidbody_component = rigidbody_component<rigidbody_kind::dynamic>;