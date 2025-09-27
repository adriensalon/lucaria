#pragma once

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <lucaria/core/layer.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {
namespace detail {
    struct dynamics_system;
}

namespace ecs {

    enum struct rigidbody_kind {
        kinematic,
        dynamic
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

        rigidbody_component& set_collide_grounds(const bool enabled = true);
        rigidbody_component& set_collide_walls(const bool enabled = true);
        rigidbody_component& set_collide_layer(const kinematic_layer layer, const bool enabled = true);

        [[nodiscard]] const std::optional<kinematic_collision>& get_ground_collision() const;
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

    // dynamic collisions ?

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

        // dynamic collisions ?

    private:
        bool _is_added = false;
        detail::fetched_container<shape> _shape = {};
        std::unique_ptr<btDefaultMotionState> _state = nullptr;
        std::unique_ptr<btRigidBody> _rigidbody = nullptr;
        glm::float32 _mass = 0.f;
        std::int16_t _group = detail::bulletgroupID_dynamic_rigidbody;
        std::int16_t _mask = detail::bulletgroupID_collider_ground | detail::bulletgroupID_collider_wall;
        friend struct detail::dynamics_system;
    };

    using kinematic_rigidbody_component = rigidbody_component<rigidbody_kind::kinematic>;
    using dynamic_rigidbody_component = rigidbody_component<rigidbody_kind::dynamic>;

}
}
