#pragma once

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <glm/gtc/quaternion.hpp>

#include <lucaria/core/layer.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {
namespace detail {
    struct motion_system;
    struct dynamics_system;
}

/// @brief 
enum struct rigidbody_type {
    passive,
    kinematic,
    dynamic
};

/// @brief 
/// @tparam Type 
template <rigidbody_type Type>
struct rigidbody_component;

template <>
struct rigidbody_component<rigidbody_type::passive> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other) = default;
    rigidbody_component& operator=(rigidbody_component&& other) = default;
    ~rigidbody_component();

    rigidbody_component& use_shape(shape& from);
    rigidbody_component& use_shape(fetched<shape>& from);

    rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);

private:
    bool _is_added = false;
    detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    friend struct detail::dynamics_system;
};

template <>
struct rigidbody_component<rigidbody_type::kinematic> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component& other) = delete;
    rigidbody_component& operator=(const rigidbody_component& other) = delete;
    rigidbody_component(rigidbody_component&& other) = default;
    rigidbody_component& operator=(rigidbody_component&& other) = default;

    rigidbody_component& use_shape(shape& from);
    rigidbody_component& use_shape(fetched<shape>& from);

    rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);

    [[nodiscard]] const std::vector<kinematic_collision>& get_collisions() const;
    [[nodiscard]] glm::vec3 get_linear_speed();
    [[nodiscard]] glm::vec3 get_angular_speed();

private:
    bool _is_added = false;
    detail::fetched_container<shape> _shape = {};
    std::unique_ptr<btPairCachingGhostObject> _ghost = nullptr;
    std::int16_t _group = 0;
    std::int16_t _mask = 0;
    std::vector<kinematic_collision> _collisions = {};
    glm::vec3 _translation_speed = glm::vec3(0);
    glm::vec3 _rotation_speed = glm::vec3(0);
    friend struct detail::dynamics_system;
};

template <>
struct rigidbody_component<rigidbody_type::dynamic> {
    rigidbody_component() = default;
    rigidbody_component(const rigidbody_component&) = delete;
    rigidbody_component& operator=(const rigidbody_component&) = delete;
    rigidbody_component(rigidbody_component&&) = default;
    rigidbody_component& operator=(rigidbody_component&&) = default;

    rigidbody_component& use_shape(shape& from);
    rigidbody_component& use_shape(fetched<shape>& from);

    rigidbody_component& set_group_layer(const collision_layer layer, const bool enable = true);
    rigidbody_component& set_mask_layer(const collision_layer layer, const bool enable = true);
    rigidbody_component& set_mass(const glm::float32 mass);
    rigidbody_component& set_friction(const glm::float32 friction);
    rigidbody_component& set_lock_angular(const glm::bvec3 lock);
    rigidbody_component& set_linear_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force);
    rigidbody_component& set_angular_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force);

    rigidbody_component& add_linear_force(const glm::vec3& force);
    rigidbody_component& add_angular_force(const glm::vec3& force);
    rigidbody_component& add_linear_impulse(const glm::vec3& impulse);
    rigidbody_component& add_angular_impulse(const glm::vec3& impulse);

    [[nodiscard]] glm::vec3 get_linear_speed();
    [[nodiscard]] glm::vec3 get_angular_speed();

private:
    bool _is_added = false;
    detail::fetched_container<shape> _shape = {};
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
    friend struct detail::motion_system;
    friend struct detail::dynamics_system;
};

using passive_rigidbody_component = rigidbody_component<rigidbody_type::passive>;
using kinematic_rigidbody_component = rigidbody_component<rigidbody_type::kinematic>;
using dynamic_rigidbody_component = rigidbody_component<rigidbody_type::dynamic>;

}
