#pragma once

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <lucaria/core/collision.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {

struct passive_rigidbody_component {
    passive_rigidbody_component() = default;
    passive_rigidbody_component(const passive_rigidbody_component& other) = delete;
    passive_rigidbody_component& operator=(const passive_rigidbody_component& other) = delete;
    passive_rigidbody_component(passive_rigidbody_component&& other) = default;
    passive_rigidbody_component& operator=(passive_rigidbody_component&& other) = default;
    ~passive_rigidbody_component();

    passive_rigidbody_component& use_shape(shape& from);
    passive_rigidbody_component& use_shape(fetched<shape>& from);

    passive_rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    passive_rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);

private:
    bool _is_added = false;
    _detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    friend struct dynamics_system;
};

struct kinematic_rigidbody_component {
    kinematic_rigidbody_component() = default;
    kinematic_rigidbody_component(const kinematic_rigidbody_component& other) = delete;
    kinematic_rigidbody_component& operator=(const kinematic_rigidbody_component& other) = delete;
    kinematic_rigidbody_component(kinematic_rigidbody_component&& other) = default;
    kinematic_rigidbody_component& operator=(kinematic_rigidbody_component&& other) = default;

    kinematic_rigidbody_component& use_shape(shape& from);
    kinematic_rigidbody_component& use_shape(fetched<shape>& from);

    kinematic_rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    kinematic_rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);

    [[nodiscard]] const std::vector<collision>& get_collisions() const;
    [[nodiscard]] glm::vec3 get_linear_speed();
    [[nodiscard]] glm::vec3 get_angular_speed();

private:
    bool _is_added = false;
    _detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btPairCachingGhostObject> _ghost = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    std::vector<collision> _collisions = {};
    glm::vec3 _translation_speed = glm::vec3(0);
    glm::vec3 _rotation_speed = glm::vec3(0);
    friend struct dynamics_system;
};

struct dynamic_rigidbody_component {
    dynamic_rigidbody_component() = default;
    dynamic_rigidbody_component(const dynamic_rigidbody_component&) = delete;
    dynamic_rigidbody_component& operator=(const dynamic_rigidbody_component&) = delete;
    dynamic_rigidbody_component(dynamic_rigidbody_component&&) = default;
    dynamic_rigidbody_component& operator=(dynamic_rigidbody_component&&) = default;

    dynamic_rigidbody_component& use_shape(shape& from);
    dynamic_rigidbody_component& use_shape(fetched<shape>& from);

    dynamic_rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    dynamic_rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);
    dynamic_rigidbody_component& set_mass(const glm::float32 mass);
    dynamic_rigidbody_component& set_friction(const glm::float32 friction);
    dynamic_rigidbody_component& set_lock_angular(const glm::bvec3 lock);
    dynamic_rigidbody_component& set_linear_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force);
    dynamic_rigidbody_component& set_angular_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force);

    dynamic_rigidbody_component& add_linear_force(const glm::vec3& force);
    dynamic_rigidbody_component& add_angular_force(const glm::vec3& force);
    dynamic_rigidbody_component& add_linear_impulse(const glm::vec3& impulse);
    dynamic_rigidbody_component& add_angular_impulse(const glm::vec3& impulse);

    [[nodiscard]] glm::vec3 get_linear_speed();
    [[nodiscard]] glm::vec3 get_angular_speed();

private:
    bool _is_added = false;
    _detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    glm::float32 _mass = 70.f;
    glm::float32 _friction = 1.f;
    glm::float32 _linear_kp = 1800.f;
    glm::float32 _linear_kd = 0.f;
    glm::float32 _linear_max_force = 6000.f;
    glm::float32 _angular_kp = 400.f;
    glm::float32 _angular_kd = 0.f;
    glm::float32 _angular_max_force = 1200.f;
    glm::float32 _angular_airborne_scale = 0.35f;
    glm::vec3 _angular_factor = glm::vec3(0, 1, 0);
    glm::vec3 _target_linear_position = glm::vec3(0);
    glm::quat _target_angular_position = glm::quat(1, glm::vec3(0));
    glm::vec3 _target_linear_velocity = glm::vec3(0);
    glm::vec3 _target_angular_velocity = glm::vec3(0);
    glm::vec3 _linear_forces = glm::vec3(0);
    glm::vec3 _angular_forces = glm::vec3(0);
    glm::vec3 _linear_impulses = glm::vec3(0);
    glm::vec3 _angular_impulses = glm::vec3(0);
    glm::vec3 _last_position = glm::vec3(0);
    glm::vec3 _translation_speed = glm::vec3(0);
    glm::vec3 _rotation_speed = glm::vec3(0);
    friend struct motion_system;
    friend struct dynamics_system;
};

}
