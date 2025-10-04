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

namespace ecs {

    enum struct rigidbody_kind {
        kinematic,
        dynamic,
        character,
    };

    template <rigidbody_kind kind_t>
    struct rigidbody_component;

    struct kinematic_collision {
        glm::float32 distance;
        glm::vec3 position;
        glm::vec3 normal;
    };

    template <>
    struct rigidbody_component<rigidbody_kind::kinematic> {
        rigidbody_component() = default;
        rigidbody_component(const rigidbody_component& other) = delete;
        rigidbody_component& operator=(const rigidbody_component& other) = delete;
        rigidbody_component(rigidbody_component&& other) = default;
        rigidbody_component& operator=(rigidbody_component&& other) = default;

        rigidbody_component& use_shape(shape& from);
        rigidbody_component& use_shape(fetched<shape>& from);

        rigidbody_component& set_collide_walls(const bool enabled = true);
        rigidbody_component& set_collide_layer(const kinematic_layer layer, const bool enabled = true);

        [[nodiscard]] const std::vector<kinematic_collision>& get_wall_collisions() const;
        [[nodiscard]] const std::vector<kinematic_collision>& get_layer_collisions(const kinematic_layer layer) const;

    private:
        bool _is_added = false;
        detail::fetched_container<shape> _shape = {};
        std::unique_ptr<btPairCachingGhostObject> _ghost = nullptr;
        bool _is_snap_ground = false;
        std::int16_t _group = detail::bulletgroupID_kinematic_rigidbody;
        std::int16_t _mask = 0;
        std::optional<kinematic_collision> _ground_collision = {};
        std::vector<kinematic_collision> _wall_collisions = {};
        std::unordered_map<kinematic_layer, std::vector<kinematic_collision>> _layer_collisions = {};
        friend struct detail::dynamics_system;
    };

    template <>
    struct rigidbody_component<rigidbody_kind::dynamic> {
        rigidbody_component() = default;
        rigidbody_component(const rigidbody_component& other) = delete;
        rigidbody_component& operator=(const rigidbody_component& other) = delete;
        rigidbody_component(rigidbody_component&& other) = default;
        rigidbody_component& operator=(rigidbody_component&& other) = default;

        rigidbody_component& use_shape(shape& from);
        rigidbody_component& use_shape(fetched<shape>& from);

        rigidbody_component& set_mass(const glm::float32 kilograms);
        rigidbody_component& set_collide_dynamics(const bool enabled = true);
        rigidbody_component& set_add_force(const glm::vec3& force);
        rigidbody_component& set_add_impulsion(const glm::vec3& force);

    private:
        bool _is_added = false;
        detail::fetched_container<shape> _shape = {};
        std::unique_ptr<btDefaultMotionState> _state = nullptr;
        std::unique_ptr<btRigidBody> _rigidbody = nullptr;
        glm::float32 _mass = 0.f;
        std::int16_t _group = detail::bulletgroupID_dynamic_rigidbody;
        std::int16_t _mask = detail::bulletgroupID_collider_wall;
        friend struct detail::dynamics_system;
    };

    template <>
    struct rigidbody_component<rigidbody_kind::character> {
        rigidbody_component() = default;
        rigidbody_component(const rigidbody_component&) = delete;
        rigidbody_component& operator=(const rigidbody_component&) = delete;
        rigidbody_component(rigidbody_component&&) = default;
        rigidbody_component& operator=(rigidbody_component&&) = default;

        rigidbody_component& use_shape(shape& from);
        rigidbody_component& use_shape(fetched<shape>& from);

        rigidbody_component& set_teleporting();
        rigidbody_component& set_mass(const glm::float32 kilograms);
        rigidbody_component& set_gravity(const glm::float32 newtons);
        // rigidbody_component& set_enable_ccd(const bool on = true);
        rigidbody_component& set_friction(const glm::float32 mu);
        rigidbody_component& set_lock_angular(const bool xlock, const bool ylock, const bool zlock);
        rigidbody_component& set_pd_xy(glm::float32 Kp, glm::float32 Kd, glm::float32 Fmax);
        rigidbody_component& set_pd_rot(glm::float32 Kp, glm::float32 Kd, glm::float32 Tmax);

    private:
        // bullet
        bool _is_added = false;
        bool _pending_teleport = true;
        detail::fetched_container<shape> _shape = {};
        std::unique_ptr<btDefaultMotionState> _state = nullptr;
        std::unique_ptr<btRigidBody> _rigidbody = nullptr;
        std::int16_t _group = detail::bulletgroupID_dynamic_rigidbody;
        std::int16_t _mask = detail::bulletgroupID_collider_wall;

        // tuning
        float _mass = 70.f;
        float _mu = 1.f;
        glm::vec3 _up = { 0, 1, 0 };
        glm::vec3 _angular_factor = { 0, 1, 0 };
        bool _use_ccd = false;
        // float _g = 9.81f;
        float _g = 0.f;

        // PD params
        float _Kp_xy = 1800.f, _Kd_xy = 0.f, _Fmax_xy = 6000.f;
        float _Kp_rot = 400.f, _Kd_rot = 0.f, _Tmax_rot = 1200.f;
        float _air_rot_scale = 0.35f;

        // targets (from animation sampling each tick)
        glm::vec3 _p_d { 0 };
        glm::quat _q_d { 1, 0, 0, 0 };
        glm::vec3 _v_d { 0 };
        glm::vec3 _w_d { 0 };

        friend struct detail::motion_system;
        friend struct detail::dynamics_system;
    };

    using kinematic_rigidbody_component = rigidbody_component<rigidbody_kind::kinematic>;
    using dynamic_rigidbody_component = rigidbody_component<rigidbody_kind::dynamic>;
    using character_rigidbody_component = rigidbody_component<rigidbody_kind::character>;

}
}
