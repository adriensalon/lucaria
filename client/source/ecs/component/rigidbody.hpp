#pragma once

#include <optional>
#include <unordered_map>
#include <vector>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include <core/layer.hpp>

enum struct rigidbody_kind {
    kinematic,
    dynamic
};

template <rigidbody_kind kind_t>
struct rigidbody_component;

struct kinematic_collision {
    float distance;
    glm::vec3 position;
    glm::vec3 normal;
};

template <>
struct rigidbody_component<rigidbody_kind::kinematic> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other);
    rigidbody_component& operator=(rigidbody_component&& other);
    ~rigidbody_component();

    rigidbody_component& motion_box();
    rigidbody_component& box(const glm::vec3& half_extents);
    rigidbody_component& capsule(const float radius, const float height);
    rigidbody_component& snap_ground(const bool enabled = true);
    rigidbody_component& glide_wall(const bool enabled = true);
    rigidbody_component& collide_layer(const kinematic_layer layer, const bool enabled = true);

    const std::optional<kinematic_collision>& get_ground_collision() const;
    const std::vector<kinematic_collision>& get_wall_collisions() const;
    const std::vector<kinematic_collision>& get_layer_collisions(const kinematic_layer layer) const;

private:
    bool _is_instanced = false;
    bool _is_motion_box = false;
    btCollisionShape* _shape = nullptr;
    btPairCachingGhostObject* _ghost = nullptr;
    float _half_height = 0.f;
    bool _is_snap_ground = false;
    short _group = bulletgroupID_kinematic_rigidbody;
    short _mask = 0;
    std::optional<kinematic_collision> _ground_collision = {};
    std::vector<kinematic_collision> _wall_collisions = {};
    std::unordered_map<kinematic_layer, std::vector<kinematic_collision>> _layer_collisions = {};
    friend struct dynamics_system;
    friend struct motion_system; // to reshape box from skinned aabb
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
    // add force
    // add impulsion

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