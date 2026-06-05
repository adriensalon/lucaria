#include <btBulletDynamicsCommon.h>

#include <lucaria/core/systems_dynamics.hpp>
#include <lucaria/core/systems_rendering.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_transform.hpp>

namespace lucaria {
namespace detail {
    namespace {

        static bool _get_manifold(btPersistentManifold** manifold, const btManifoldArray& array, const int index)
        {
            *manifold = array[index];
            if (*manifold) {
                return true;
            }
            return false;
        }

        static std::vector<collision> _get_collisions(const btPersistentManifold* manifold, btGhostObject* ghost)
        {
            std::vector<collision> _collisions;
            for (int _contact_index = 0; _contact_index < manifold->getNumContacts(); ++_contact_index) {
                const btManifoldPoint& _contact_point = manifold->getContactPoint(_contact_index);
                if (_contact_point.getDistance() > 0.f) {
                    continue;
                }
                const bool _ghost_is_first = (manifold->getBody0() == ghost);
                const btVector3 _position_other = _ghost_is_first ? _contact_point.getPositionWorldOnB() : _contact_point.getPositionWorldOnA();
                const btVector3 _normal_second = _contact_point.m_normalWorldOnB.normalized();
                const btVector3 _normal_other = _ghost_is_first ? -_normal_second : _normal_second;
                collision& _collision = _collisions.emplace_back();
                _collision.distance = _contact_point.getDistance();
                _collision.position = convert(_position_other);
                _collision.normal = convert(_normal_other);
            }
            return _collisions;
        }

        static float32x3 _forward_xy(const system_dynamics& dynamics, const glm::quat& rotation)
        {
            const float32x3 _force = project_on_plane(rotation * dynamics.world_forward, dynamics.world_up);
            const float32 _force_length = glm::length(_force);
            return (_force_length > 0.f) ? (_force * (1.f / _force_length)) : dynamics.world_forward;
        };

        static bool _is_grounded(const system_dynamics& dynamics, btRigidBody* rigidbody, const std::int16_t mask, const btScalar maximum_slope = btCos(btRadians(160.0f)), const btScalar maximum_distance = btScalar(0.5))
        {
            // const btTransform _transform = rigidbody->getInterpolationWorldTransform();
            const btTransform _transform = rigidbody->getWorldTransform();
            const btVector3 _from = _transform.getOrigin();
            const btVector3 _to = _from - convert_bullet(dynamics.world_up) * (maximum_distance + 1.f);
            btCollisionWorld::ClosestRayResultCallback _raycast_callback(_from, _to);
            _raycast_callback.m_collisionFilterMask = mask;
            dynamics.dynamics_world->rayTest(_from, _to, _raycast_callback);
            if (_raycast_callback.hasHit()) {
                const btVector3 _hit_normal = _raycast_callback.m_hitNormalWorld.normalized();
                if (_hit_normal.dot(convert_bullet(dynamics.world_up)) > maximum_slope) {
                    return true;
                }
            }
            return false;
        }

    }

    system_dynamics::system_dynamics()
    {
        collision_configuration = new btDefaultCollisionConfiguration();
        collision_dispatcher = new btCollisionDispatcher(collision_configuration);
        overlapping_pair_cache = new btDbvtBroadphase();
        overlapping_pair_cache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
        constraint_solver = new btSequentialImpulseConstraintSolver();
        dynamics_world = new btDiscreteDynamicsWorld(collision_dispatcher, overlapping_pair_cache, constraint_solver, collision_configuration);
        dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
    }

    void system_dynamics::update_step_simulation(manager_window& window, manager_scenes& scenes)
    {
        // update collider transforms
        scenes.each_view<component_rigidbody_passive>(exclude<component_transform>, [](component_rigidbody_passive& collider) {
            const bool _collected = collider._shape.has_value(); // colliders can have no transform
            (void)_collected;
        });

        scenes.each_view<component_rigidbody_passive, component_transform>([](component_rigidbody_passive& collider, component_transform& transform) {
            if (!collider._shape) {
                return;
            }
            // collider._rigidbody->setInterpolationWorldTransform(convert_bullet(transform._transform));
            collider._rigidbody->setWorldTransform(convert_bullet(transform._transform));
        });

        // update kinematic rigidbody transforms
        scenes.each_view<component_rigidbody_kinematic, component_transform>([](component_rigidbody_kinematic& rigidbody, component_transform& transform) {
            if (!rigidbody._shape) {
                return;
            }
            // rigidbody._ghost->setInterpolationWorldTransform(convert_bullet(transform._transform));
            rigidbody._ghost->setWorldTransform(convert_bullet(transform._transform));
        });

        // apply dynamic rigidbody forces
        const float32 _delta_time = static_cast<float32>(window.time_delta_seconds);
        scenes.each_view<component_rigidbody_dynamic>([&](component_rigidbody_dynamic& rigidbody) {
            if (!rigidbody._shape) {
                return;
            }
            btRigidBody* _bullet_rigidbody = rigidbody._rigidbody.get();
            // const glm::mat4 _bullet_transform = convert(_bullet_rigidbody->getInterpolationWorldTransform());
            const glm::mat4 _bullet_transform = convert(_bullet_rigidbody->getWorldTransform());
            const float32x3 _bullet_linear_position = float32x3(_bullet_transform[3]);
            // const float32x3 _bullet_linear_velocity = convert(_bullet_rigidbody->getInterpolationLinearVelocity());
            const float32x3 _bullet_linear_velocity = convert(_bullet_rigidbody->getLinearVelocity());
            const glm::quat _bullet_angular_position = glm::quat_cast(_bullet_transform);
            // const float32x3 _bullet_angular_velocity = convert(_bullet_rigidbody->getInterpolationAngularVelocity());
            const float32x3 _bullet_angular_velocity = convert(_bullet_rigidbody->getAngularVelocity());

            // append linear PD forces
            const float32x3 _error_position_xy = project_on_plane(rigidbody._target_linear_position - _bullet_linear_position, world_up);
            const float32x3 _error_velocity_xy = project_on_plane(rigidbody._target_linear_velocity - _bullet_linear_velocity, world_up);
            float32x3 _force_xy = rigidbody._linear_kp * _error_position_xy + rigidbody._linear_kd * _error_velocity_xy;
            const float32 _force_length_xy = glm::length(_force_xy);
            if (_force_length_xy > rigidbody._linear_max_force && _force_length_xy > 0.f) {
                _force_xy *= rigidbody._linear_max_force / _force_length_xy;
            }
            rigidbody._linear_forces += _force_xy;

            // append angular PD forces
            const float32x3 _forward_now = _forward_xy(*this, _bullet_angular_position);
            const float32x3 _forward_destination = _forward_xy(*this, rigidbody._target_angular_position);
            const float32 _cos_error_position_yaw = glm::clamp(glm::dot(_forward_now, _forward_destination), -1.f, 1.f);
            const float32 _sin_error_position_yaw = glm::dot(glm::cross(_forward_now, _forward_destination), world_up);
            const float32 _error_position_yaw = std::atan2(_sin_error_position_yaw, _cos_error_position_yaw);
            const float32 _error_velocity_yaw = glm::dot(rigidbody._target_angular_velocity, world_up) - glm::dot(_bullet_angular_velocity, world_up);
            const bool _grounded = _is_grounded(*this, _bullet_rigidbody, rigidbody._mask);
            const float32 _angular_kp = rigidbody._angular_kp * (_grounded ? 1.f : rigidbody._angular_airborne_scale);
            const float32 _angular_kd = rigidbody._angular_kd * (_grounded ? 1.f : rigidbody._angular_airborne_scale);
            float32x3 _torque_yaw = world_up * (_angular_kp * _error_position_yaw + _angular_kd * _error_velocity_yaw);
            const float32 _torque_yaw_length = glm::length(_torque_yaw);
            if (_torque_yaw_length > rigidbody._angular_max_force && _torque_yaw_length > 0.f) {
                _torque_yaw *= rigidbody._angular_max_force / _torque_yaw_length;
            }
            rigidbody._angular_forces += _torque_yaw;

            // submit all forces
            _bullet_rigidbody->applyCentralForce(convert_bullet(rigidbody._linear_forces));
            _bullet_rigidbody->applyTorque(convert_bullet(rigidbody._angular_forces));
            _bullet_rigidbody->applyCentralImpulse(convert_bullet(rigidbody._linear_impulses));
            _bullet_rigidbody->applyTorqueImpulse(convert_bullet(rigidbody._angular_impulses));
            _bullet_rigidbody->activate(true);
            rigidbody._linear_forces = float32x3(0);
            rigidbody._angular_forces = float32x3(0);
            rigidbody._linear_impulses = float32x3(0);
            rigidbody._angular_impulses = float32x3(0);
        });

        // step once each frame no substep no interpolation
        dynamics_world->stepSimulation(_delta_time, 0);
    }

    void system_dynamics::update_compute_collisions(manager_scenes& scenes)
    {
        // collect collisions from kinematic rigidbodies
        btManifoldArray _manifold_array;
        scenes.each_view<component_transform, component_rigidbody_kinematic>([&](component_transform& transform, component_rigidbody_kinematic& rigidbody) {
            rigidbody._collisions.clear();
            btBroadphasePairArray& _pair_array = rigidbody._ghost->getOverlappingPairCache()->getOverlappingPairArray();
            for (int _pair_index = 0; _pair_index < _pair_array.size(); _pair_index++) {
                _manifold_array.clear();
                btBroadphasePair* _collision_pair = dynamics_world->getPairCache()->findPair(_pair_array[_pair_index].m_pProxy0, _pair_array[_pair_index].m_pProxy1);
                if (!_collision_pair) {
                    continue;
                }
                btCollisionAlgorithm* _collision_algorithm = _collision_pair->m_algorithm;
                if (!_collision_algorithm) {
                    continue;
                }
                _collision_algorithm->getAllContactManifolds(_manifold_array);
                for (int _manifold_index = 0; _manifold_index < _manifold_array.size(); _manifold_index++) {
                    btPersistentManifold* _manifold;
                    if (!_get_manifold(&_manifold, _manifold_array, _manifold_index)) {
                        continue;
                    }
                    rigidbody._collisions = _get_collisions(_manifold, rigidbody._ghost.get());
                }
            }
            // rigidbody._translation_speed = convert(rigidbody._ghost->getInterpolationLinearVelocity());
            // rigidbody._translation_speed = convert(rigidbody._ghost->getLinearVelocity());
            // rigidbody._rotation_speed = convert(rigidbody._ghost->getInterpolationAngularVelocity());
            // rigidbody._rotation_speed = convert(rigidbody._ghost->getAngularVelocity());
        });

        // set transform from dynamic rigidbodies
        scenes.each_view<component_transform, component_rigidbody_dynamic>([](component_transform& transform, component_rigidbody_dynamic& rigidbody) {
            // const glm::mat4 _transform = convert(rigidbody._rigidbody->getInterpolationWorldTransform());
            const glm::mat4 _transform = convert(rigidbody._rigidbody->getWorldTransform());
            const glm::mat4 _center_to_feet = rigidbody._shape._cached->fetched.value().center_to_feet;
            transform.set_transform_warp(_transform * _center_to_feet);
            rigidbody._last_position = transform.get_position();
            // rigidbody._translation_speed = convert(rigidbody._rigidbody->getInterpolationLinearVelocity());
            rigidbody._translation_speed = convert(rigidbody._rigidbody->getLinearVelocity());
            // rigidbody._rotation_speed = convert(rigidbody._rigidbody->getInterpolationAngularVelocity());
            rigidbody._rotation_speed = convert(rigidbody._rigidbody->getAngularVelocity());
        });
    }

    void system_dynamics::update_collect_debug_guizmos(system_rendering& rendering, manager_scenes& scenes)
    {
#if defined(LUCARIA_DEBUG)
        scenes.each_view<component_transform, component_rigidbody_kinematic>([&](component_transform& transform, component_rigidbody_kinematic& rigidbody) {
            for (const lucaria::collision& _collision : rigidbody._collisions) {
                const float32x3 _from = _collision.position;
                const float32x3 _to = _collision.position + 0.2f * glm::normalize(_collision.normal);
                rendering.guizmo_draw.drawLine(convert_bullet(_from), convert_bullet(_to), btVector3(1, 0, 1)); // purple
            }
        });
#endif
    }

    std::optional<collision> system_dynamics::raycast(system_rendering& rendering, const float32x3& from, const float32x3& to)
    {
        const btVector3 _from = convert_bullet(from);
        const btVector3 _to = convert_bullet(to);
        btCollisionWorld::ClosestRayResultCallback _raycallback(_from, _to);
        dynamics_world->rayTest(_from, _to, _raycallback);
#if defined(LUCARIA_DEBUG)
        rendering.guizmo_draw.drawLine(_from, _to, btVector3(1, 0, 1)); // purple
#endif
        if (_raycallback.hasHit()) {
            collision _collision;
            _collision.position = convert(_raycallback.m_hitPointWorld);
            _collision.normal = glm::normalize(convert(_raycallback.m_hitNormalWorld));
            _collision.distance = glm::distance(from, _collision.position);
#if defined(LUCARIA_DEBUG)
            rendering.guizmo_draw.drawLine(_raycallback.m_hitPointWorld, _raycallback.m_hitPointWorld + _raycallback.m_hitNormalWorld * 0.2f, btVector3(1, 1, 1)); // white
#endif
            return _collision;
        }
        return std::nullopt;
    }

    void system_dynamics::set_world_gravity(const float32 newtons)
    {
        dynamics_world->setGravity(convert_bullet(-world_up * newtons));
    }

}
}
