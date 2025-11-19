#include <btBulletDynamicsCommon.h>

#include <lucaria/component/collider.hpp>
#include <lucaria/component/rigidbody.hpp>
#include <lucaria/component/transform.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    btDiscreteDynamicsWorld* _dynamics_world = nullptr;

#if LUCARIA_GUIZMO
    extern void _draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

}

namespace {

    static const glm::vec3 _world_up = glm::vec3(0, 1, 0);
    static const glm::vec3 _world_forward = glm::vec3(0, 0, 1);
    static btDefaultCollisionConfiguration* _collision_configuration = nullptr;
    static btCollisionDispatcher* _collision_dispatcher = nullptr;
    static btBroadphaseInterface* _overlapping_pair_cache = nullptr;
    static btSequentialImpulseConstraintSolver* _constraint_solver = nullptr;

    static bool _setup_bullet_worlds()
    {
        _collision_configuration = new btDefaultCollisionConfiguration();
        _collision_dispatcher = new btCollisionDispatcher(_collision_configuration);
        _overlapping_pair_cache = new btDbvtBroadphase();
        _overlapping_pair_cache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
        _constraint_solver = new btSequentialImpulseConstraintSolver();
        detail::_dynamics_world = new btDiscreteDynamicsWorld(_collision_dispatcher, _overlapping_pair_cache, _constraint_solver, _collision_configuration);
        detail::_dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
        return true;
    }

    static bool _is_bullet_worlds_setup = _setup_bullet_worlds();

    static bool _get_manifold(btPersistentManifold** manifold, const btManifoldArray& array, const int index)
    {
        *manifold = array[index];
        if (*manifold) {
            return true;
        }
        return false;
    }

    static bool _get_other_object(btCollisionObject** other_object, const btPersistentManifold* manifold, const btPairCachingGhostObject* ghost)
    {
        btCollisionObject* _obj_A = const_cast<btCollisionObject*>(manifold->getBody0());
        btCollisionObject* _obj_B = const_cast<btCollisionObject*>(manifold->getBody1());
        if (_obj_A == ghost) {
            *other_object = _obj_B;
            return true;
        } else if (_obj_B == ghost) {
            *other_object = _obj_A;
            return true;
        }
        return false;
    }

    static bool _get_other_group(std::int16_t& other_group, std::int16_t& other_mask, const btCollisionObject* other_object)
    {
        const btBroadphaseProxy* _proxy = other_object->getBroadphaseHandle();
        if (_proxy) {
            other_group = _proxy->m_collisionFilterGroup;
            other_mask = _proxy->m_collisionFilterMask;
            return true;
        }
        return false;
    }

    static std::vector<kinematic_collision> _get_collisions(const btPersistentManifold* manifold, btGhostObject* ghost)
    {
        std::vector<kinematic_collision> _collisions;
        for (int _contact_index = 0; _contact_index < manifold->getNumContacts(); ++_contact_index) {
            const btManifoldPoint& _contact_point = manifold->getContactPoint(_contact_index);
            if (_contact_point.getDistance() > 0.f) {
                continue;
            }
            const bool _ghost_is_first = (manifold->getBody0() == ghost);
            const btVector3 _position_other = _ghost_is_first ? _contact_point.getPositionWorldOnB() : _contact_point.getPositionWorldOnA();
            const btVector3 _normal_second = _contact_point.m_normalWorldOnB.normalized();
            const btVector3 _normal_other = _ghost_is_first ? -_normal_second : _normal_second;
            kinematic_collision& _collision = _collisions.emplace_back();
            _collision.distance = _contact_point.getDistance();
            _collision.position = detail::convert(_position_other);
            _collision.normal = detail::convert(_normal_other);
        }
        return _collisions;
    }

    static glm::vec3 _forward_xy(const glm::quat& rotation)
    {
        const glm::vec3 _force = detail::project_on_plane(rotation * _world_forward, _world_up);
        const glm::float32 _force_length = glm::length(_force);
        return (_force_length > 0.f) ? (_force * (1.f / _force_length)) : _world_forward;
    };

    static bool _is_grounded(btRigidBody* rigidbody, const std::int16_t mask, const btScalar maximum_slope = btCos(btRadians(160.0f)), const btScalar maximum_distance = btScalar(0.5))
    {
        const btTransform _transform = rigidbody->getWorldTransform();
        const btVector3 _from = _transform.getOrigin();
        const btVector3 _to = _from - detail::convert_bullet(_world_up) * (maximum_distance + 1.f);
        btCollisionWorld::ClosestRayResultCallback _raycast_callback(_from, _to);
        _raycast_callback.m_collisionFilterMask = mask;
        detail::_dynamics_world->rayTest(_from, _to, _raycast_callback);
        if (_raycast_callback.hasHit()) {
            const btVector3 _hit_normal = _raycast_callback.m_hitNormalWorld.normalized();
            if (_hit_normal.dot(detail::convert_bullet(_world_up)) > maximum_slope) {
                return true;
            }
        }
        return false;
    }

}

std::optional<raycast_collision> raycast(const glm::vec3& from, const glm::vec3& to)
{
    const btVector3 _from = detail::convert_bullet(from);
    const btVector3 _to = detail::convert_bullet(to);
    btCollisionWorld::ClosestRayResultCallback _raycallback(_from, _to);
    detail::_dynamics_world->rayTest(_from, _to, _raycallback);
#if LUCARIA_GUIZMO
    detail::_draw_guizmo_line(_from, _to, btVector3(1, 0, 1)); // purple
#endif
    if (_raycallback.hasHit()) {
        raycast_collision _collision;
        _collision.position = detail::convert(_raycallback.m_hitPointWorld);
        _collision.normal = glm::normalize(detail::convert(_raycallback.m_hitNormalWorld));
#if LUCARIA_GUIZMO
        detail::_draw_guizmo_line(_raycallback.m_hitPointWorld, _raycallback.m_hitPointWorld + _raycallback.m_hitNormalWorld * 0.2f, btVector3(1, 1, 1)); // white
#endif
        return _collision;
    }
    return std::nullopt;
}

void set_world_gravity(const glm::vec3& newtons)
{
    detail::_dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

namespace detail {

    void dynamics_system::step_simulation()
    {
        // update collider transforms
        detail::each_scene([&](entt::registry& scene) {
            scene.view<collider_component>(entt::exclude<transform_component>).each([](collider_component& collider) {
                collider._shape.has_value(); // colliders can have no transform
            });
            scene.view<collider_component, transform_component>().each([](collider_component& collider, transform_component& transform) {
                if (!collider._shape.has_value()) {
                    return;
                }
                collider._rigidbody->setWorldTransform(convert_bullet(transform._transform));
            });
        });

        // update kinematic rigidbody transforms
        detail::each_scene([&](entt::registry& scene) {
            scene.view<kinematic_rigidbody_component, transform_component>().each([](kinematic_rigidbody_component& rigidbody, transform_component& transform) {
                if (!rigidbody._shape.has_value()) {
                    return;
                }
                rigidbody._ghost->setWorldTransform(convert_bullet(transform._transform));
            });
        });

        // apply dynamic rigidbody forces
        const glm::float32 _delta_time = static_cast<glm::float32>(get_time_delta());
        detail::each_scene([&](entt::registry& scene) {
            scene.view<dynamic_rigidbody_component>().each([&](dynamic_rigidbody_component& rigidbody) {
                if (!rigidbody._shape.has_value()) {
                    return;
                }
                btRigidBody* _bullet_rigidbody = rigidbody._rigidbody.get();
                const glm::mat4 _bullet_transform = convert(_bullet_rigidbody->getWorldTransform());
                const glm::vec3 _bullet_linear_position = glm::vec3(_bullet_transform[3]);
                const glm::vec3 _bullet_linear_velocity = convert(_bullet_rigidbody->getLinearVelocity());
                const glm::quat _bullet_angular_position = glm::quat_cast(_bullet_transform);
                const glm::vec3 _bullet_angular_velocity = convert(_bullet_rigidbody->getAngularVelocity());

                // append linear PD forces
                const glm::vec3 _error_position_xy = project_on_plane(rigidbody._target_linear_position - _bullet_linear_position, _world_up);
                const glm::vec3 _error_velocity_xy = project_on_plane(rigidbody._target_linear_velocity - _bullet_linear_velocity, _world_up);
                glm::vec3 _force_xy = rigidbody._linear_kp * _error_position_xy + rigidbody._linear_kd * _error_velocity_xy;
                const glm::float32 _force_length_xy = glm::length(_force_xy);
                if (_force_length_xy > glm::pow(rigidbody._linear_max_force, 2.f)) {
                    _force_xy *= rigidbody._linear_max_force / glm::sqrt(_force_length_xy);
                }
                rigidbody._linear_forces += _force_xy;

                // append angular PD forces
                const glm::vec3 _forward_now = _forward_xy(_bullet_angular_position);
                const glm::vec3 _forward_destination = _forward_xy(rigidbody._target_angular_position);
                const glm::float32 _cos_error_position_yaw = glm::clamp(glm::dot(_forward_now, _forward_destination), -1.f, 1.f);
                const glm::float32 _sin_error_position_yaw = glm::dot(glm::cross(_forward_now, _forward_destination), _world_up);
                const glm::float32 _error_position_yaw = std::atan2(_sin_error_position_yaw, _cos_error_position_yaw);
                const glm::float32 _error_velocity_yaw = glm::dot(rigidbody._target_angular_velocity, _world_up) - glm::dot(_bullet_angular_velocity, _world_up);
                const bool _grounded = _is_grounded(_bullet_rigidbody, rigidbody._mask);
                const glm::float32 _angular_kp = rigidbody._angular_kp * (_grounded ? 1.f : rigidbody._angular_airborne_scale);
                const glm::float32 _angular_kd = rigidbody._angular_kd * (_grounded ? 1.f : rigidbody._angular_airborne_scale);
                glm::vec3 _torque_yaw = _world_up * (_angular_kp * _error_position_yaw + _angular_kd * _error_velocity_yaw);
                const glm::float32 _torque_yaw_length = glm::length(_torque_yaw);
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
                rigidbody._linear_forces = glm::vec3(0);
                rigidbody._angular_forces = glm::vec3(0);
                rigidbody._linear_impulses = glm::vec3(0);
                rigidbody._angular_impulses = glm::vec3(0);
            });
        });

        // step once each frame no substep no interpolation
        detail::_dynamics_world->stepSimulation(_delta_time, 0);
    }

    void dynamics_system::compute_collisions()
    {
        // collect collisions from kinematic rigidbodies
        btManifoldArray _manifold_array;
        detail::each_scene([&](entt::registry& scene) {
            scene.view<transform_component, kinematic_rigidbody_component>().each([&](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
                rigidbody._world_collisions.clear();
                rigidbody._layer_collisions.clear();
                btBroadphasePairArray& _pair_array = rigidbody._ghost->getOverlappingPairCache()->getOverlappingPairArray();
                for (int _pair_index = 0; _pair_index < _pair_array.size(); _pair_index++) {
                    _manifold_array.clear();
                    btBroadphasePair* _collision_pair = detail::_dynamics_world->getPairCache()->findPair(_pair_array[_pair_index].m_pProxy0, _pair_array[_pair_index].m_pProxy1);
                    if (!_collision_pair) {
                        continue;
                    }
                    btCollisionAlgorithm* _collision_algorithm = _collision_pair->m_algorithm;
                    if (!_collision_algorithm) {
                        continue;
                    }
                    _collision_algorithm->getAllContactManifolds(_manifold_array);
                    for (int _manifold_index = 0; _manifold_index < _manifold_array.size(); _manifold_index++) {
                        std::int16_t _other_group;
                        std::int16_t _other_mask;
                        btPersistentManifold* _manifold;
                        btCollisionObject* _other_object;
                        if (!_get_manifold(&_manifold, _manifold_array, _manifold_index)) {
                            continue;
                        }
                        if (!_get_other_object(&_other_object, _manifold, rigidbody._ghost.get())) {
                            continue;
                        }
                        if (!_get_other_group(_other_group, _other_mask, _other_object)) {
                            continue;
                        }
                        if ((rigidbody._group & _other_mask) && (_other_group & rigidbody._mask)) {
                            // rigidbody._world_collisions = _get_collisions(_manifold, rigidbody._ghost.get());
                        } else {
                            // rigidbody._layer_collisions[static_cast<collision_layer>(_other_group)] = _get_collisions(_manifold, rigidbody._ghost.get());
                        }
                    }
                }
                rigidbody._translation_speed = convert(rigidbody._ghost->getInterpolationLinearVelocity());
                rigidbody._rotation_speed = convert(rigidbody._ghost->getInterpolationAngularVelocity());
            });
        });

        // set transform from dynamic rigidbodies
        detail::each_scene([&](entt::registry& scene) {
            scene.view<transform_component, dynamic_rigidbody_component>().each([](transform_component& transform, dynamic_rigidbody_component& rigidbody) {
                const glm::mat4 _transform = convert(rigidbody._rigidbody->getWorldTransform());
                const glm::mat4 _center_to_feet = rigidbody._shape.value().get_center_to_feet();
                transform.set_transform_warp(_transform * _center_to_feet);
                rigidbody._last_position = transform.get_position();
                rigidbody._translation_speed = convert(rigidbody._rigidbody->getLinearVelocity());
                rigidbody._rotation_speed = convert(rigidbody._rigidbody->getAngularVelocity());
            });
        });
    }

    void dynamics_system::collect_debug_guizmos()
    {
#if LUCARIA_GUIZMO
        detail::each_scene([&](entt::registry& scene) {
            scene.view<transform_component, kinematic_rigidbody_component>().each([&](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
                for (const lucaria::kinematic_collision& _collision : rigidbody._world_collisions) {
                    const glm::vec3 _from = _collision.position;
                    const glm::vec3 _to = _collision.position + 0.2f * glm::normalize(_collision.normal);
                    _draw_guizmo_line(convert_bullet(_from), convert_bullet(_to), btVector3(1, 0, 1)); // purple
                }
            });
        });
#endif
    }
}
}
