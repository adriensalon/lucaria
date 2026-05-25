#pragma once

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <lucaria/core/utils_collision.hpp>
#include <lucaria/public/handle_shape.hpp>

namespace lucaria {

struct context_dynamics;

struct component_rigidbody_passive {
    component_rigidbody_passive(context_dynamics& dynamics);
    component_rigidbody_passive(const component_rigidbody_passive& other) = delete;
    component_rigidbody_passive& operator=(const component_rigidbody_passive& other) = delete;
    component_rigidbody_passive(component_rigidbody_passive&& other) = default;
    component_rigidbody_passive& operator=(component_rigidbody_passive&& other) = default;
    ~component_rigidbody_passive();

    component_rigidbody_passive& use_shape(const handle_shape shape);

    component_rigidbody_passive& set_group_layer(const collision_layer layer, const bool enable = true);
    component_rigidbody_passive& set_mask_layer(const collision_layer layer, const bool enable = true);

private:
    bool _is_added = false;
    btDiscreteDynamicsWorld* _dynamics_world = nullptr;
    handle_shape _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    int16 _group = 0;
    int16 _mask = 0;

    template <typename Archive>
    void save(Archive& archive) const
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
    }

    template <typename Archive>
    void load(Archive& archive)
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
        use_shape(_shape);
    }

    friend struct detail::system_dynamics;
    friend class cereal::access;
};

struct component_rigidbody_kinematic {
    component_rigidbody_kinematic(context_dynamics& dynamics);
    component_rigidbody_kinematic(const component_rigidbody_kinematic& other) = delete;
    component_rigidbody_kinematic& operator=(const component_rigidbody_kinematic& other) = delete;
    component_rigidbody_kinematic(component_rigidbody_kinematic&& other) = default;
    component_rigidbody_kinematic& operator=(component_rigidbody_kinematic&& other) = default;

    component_rigidbody_kinematic& use_shape(const handle_shape shape);

    component_rigidbody_kinematic& set_group_layer(const collision_layer layer, const bool enable = true);
    component_rigidbody_kinematic& set_mask_layer(const collision_layer layer, const bool enable = true);

    [[nodiscard]] const std::vector<collision>& get_collisions() const;
    [[nodiscard]] float32x3 get_linear_speed();
    [[nodiscard]] float32x3 get_angular_speed();

private:
    bool _is_added = false;
    btDiscreteDynamicsWorld* _dynamics_world = nullptr;
    handle_shape _shape = {};
    std::unique_ptr<btPairCachingGhostObject> _ghost = nullptr;
    int16 _group = 0;
    int16 _mask = 0;
    std::vector<collision> _collisions = {};
    float32x3 _translation_speed = float32x3(0);
    float32x3 _rotation_speed = float32x3(0);

    template <typename Archive>
    void save(Archive& archive) const
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
        archive(cereal::make_nvp("translation_speed", _translation_speed));
        archive(cereal::make_nvp("rotation_speed", _rotation_speed));
    }

    template <typename Archive>
    void load(Archive& archive)
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
        archive(cereal::make_nvp("translation_speed", _translation_speed));
        archive(cereal::make_nvp("rotation_speed", _rotation_speed));
        use_shape(_shape);
    }

    friend struct detail::system_dynamics;
    friend class cereal::access;
};

struct component_rigidbody_dynamic {
    component_rigidbody_dynamic(context_dynamics& dynamics);
    component_rigidbody_dynamic(const component_rigidbody_dynamic&) = delete;
    component_rigidbody_dynamic& operator=(const component_rigidbody_dynamic&) = delete;
    component_rigidbody_dynamic(component_rigidbody_dynamic&&) = default;
    component_rigidbody_dynamic& operator=(component_rigidbody_dynamic&&) = default;

    component_rigidbody_dynamic& use_shape(const handle_shape shape);

    component_rigidbody_dynamic& set_group_layer(const collision_layer layer, const bool enable = true);
    component_rigidbody_dynamic& set_mask_layer(const collision_layer layer, const bool enable = true);
    component_rigidbody_dynamic& set_mass(const float32 mass);
    component_rigidbody_dynamic& set_friction(const float32 friction);
    component_rigidbody_dynamic& set_lock_angular(const glm::bvec3 lock);
    component_rigidbody_dynamic& set_linear_pd(const float32 kp, const float32 kd, const float32 max_force);
    component_rigidbody_dynamic& set_angular_pd(const float32 kp, const float32 kd, const float32 max_force);

    component_rigidbody_dynamic& add_linear_force(const float32x3& force);
    component_rigidbody_dynamic& add_angular_force(const float32x3& force);
    component_rigidbody_dynamic& add_linear_impulse(const float32x3& impulse);
    component_rigidbody_dynamic& add_angular_impulse(const float32x3& impulse);

    [[nodiscard]] float32x3 get_linear_speed();
    [[nodiscard]] float32x3 get_angular_speed();

private:
    bool _is_added = false;
    btDiscreteDynamicsWorld* _dynamics_world = nullptr;
    handle_shape _shape = {};
    std::unique_ptr<btDefaultMotionState> _state = nullptr;
    std::unique_ptr<btRigidBody> _rigidbody = nullptr;
    int16 _group = 0;
    int16 _mask = 0;
    float32 _mass = 70.f;
    float32 _friction = 1.f;
    float32 _linear_kp = 1800.f;
    float32 _linear_kd = 0.f;
    float32 _linear_max_force = 6000.f;
    float32 _angular_kp = 400.f;
    float32 _angular_kd = 0.f;
    float32 _angular_max_force = 1200.f;
    float32 _angular_airborne_scale = 0.35f;
    float32x3 _angular_factor = float32x3(0, 1, 0);
    float32x3 _target_linear_position = float32x3(0);
    float32quat _target_angular_position = float32quat(1, float32x3(0));
    float32x3 _target_linear_velocity = float32x3(0);
    float32x3 _target_angular_velocity = float32x3(0);
    float32x3 _linear_forces = float32x3(0);
    float32x3 _angular_forces = float32x3(0);
    float32x3 _linear_impulses = float32x3(0);
    float32x3 _angular_impulses = float32x3(0);
    float32x3 _last_position = float32x3(0);
    float32x3 _translation_speed = float32x3(0);
    float32x3 _rotation_speed = float32x3(0);

    template <typename Archive>
    void save(Archive& archive) const
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
        archive(cereal::make_nvp("mass", _mass));
        archive(cereal::make_nvp("friction", _friction));
        archive(cereal::make_nvp("linear_kp", _linear_kp));
        archive(cereal::make_nvp("linear_kd", _linear_kd));
        archive(cereal::make_nvp("linear_max_force", _linear_max_force));
        archive(cereal::make_nvp("angular_kp", _angular_kp));
        archive(cereal::make_nvp("angular_kd", _angular_kd));
        archive(cereal::make_nvp("angular_max_force", _angular_max_force));
        archive(cereal::make_nvp("angular_airborne_scale", _angular_airborne_scale));
        archive(cereal::make_nvp("angular_factor", _angular_factor));
        archive(cereal::make_nvp("target_linear_position", _target_linear_position));
        archive(cereal::make_nvp("target_angular_position", _target_angular_position));
        archive(cereal::make_nvp("target_linear_velocity", _target_linear_velocity));
        archive(cereal::make_nvp("target_angular_velocity", _target_angular_velocity));
        archive(cereal::make_nvp("linear_forces", _linear_forces));
        archive(cereal::make_nvp("angular_forces", _angular_forces));
        archive(cereal::make_nvp("linear_impulses", _linear_impulses));
        archive(cereal::make_nvp("angular_impulses", _angular_impulses));
        archive(cereal::make_nvp("last_position", _last_position));
        archive(cereal::make_nvp("translation_speed", _translation_speed));
        archive(cereal::make_nvp("rotation_speed", _rotation_speed));
    }

    template <typename Archive>
    void load(Archive& archive)
    {
        archive(cereal::make_nvp("shape", _shape));
        archive(cereal::make_nvp("group", _group));
        archive(cereal::make_nvp("mask", _mask));
        archive(cereal::make_nvp("mass", _mass));
        archive(cereal::make_nvp("friction", _friction));
        archive(cereal::make_nvp("linear_kp", _linear_kp));
        archive(cereal::make_nvp("linear_kd", _linear_kd));
        archive(cereal::make_nvp("linear_max_force", _linear_max_force));
        archive(cereal::make_nvp("angular_kp", _angular_kp));
        archive(cereal::make_nvp("angular_kd", _angular_kd));
        archive(cereal::make_nvp("angular_max_force", _angular_max_force));
        archive(cereal::make_nvp("angular_airborne_scale", _angular_airborne_scale));
        archive(cereal::make_nvp("angular_factor", _angular_factor));
        archive(cereal::make_nvp("target_linear_position", _target_linear_position));
        archive(cereal::make_nvp("target_angular_position", _target_angular_position));
        archive(cereal::make_nvp("target_linear_velocity", _target_linear_velocity));
        archive(cereal::make_nvp("target_angular_velocity", _target_angular_velocity));
        archive(cereal::make_nvp("linear_forces", _linear_forces));
        archive(cereal::make_nvp("angular_forces", _angular_forces));
        archive(cereal::make_nvp("linear_impulses", _linear_impulses));
        archive(cereal::make_nvp("angular_impulses", _angular_impulses));
        archive(cereal::make_nvp("last_position", _last_position));
        archive(cereal::make_nvp("translation_speed", _translation_speed));
        archive(cereal::make_nvp("rotation_speed", _rotation_speed));
        use_shape(_shape);
    }

    friend struct detail::system_motion;
    friend struct detail::system_dynamics;
    friend class cereal::access;
};

}
