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

    static btDefaultCollisionConfiguration* _collision_configuration = nullptr;
    static btCollisionDispatcher* _collision_dispatcher = nullptr;
    static btBroadphaseInterface* _overlapping_pair_cache = nullptr;
    static btSequentialImpulseConstraintSolver* _constraint_solver = nullptr;

    static bool setup_bullet_worlds()
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

    static bool is_bullet_worlds_setup = setup_bullet_worlds();

    static bool get_manifold(btPersistentManifold** manifold, const btManifoldArray& array, const int index)
    {
        *manifold = array[index];
        if (*manifold) {
            return true;
        }
        return false;
    }

    static bool get_other_object(btCollisionObject** other_object, const btPersistentManifold* manifold, const btPairCachingGhostObject* ghost)
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

    static bool get_other_group(std::int16_t& other_group, const btCollisionObject* other_object)
    {
        const btBroadphaseProxy* _proxy = other_object->getBroadphaseHandle();
        if (_proxy) {
            other_group = _proxy->m_collisionFilterGroup;
            return true;
        }
        return false;
    }

    static std::vector<kinematic_collision> get_collisions(const btPersistentManifold* manifold, btGhostObject* ghost)
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

    static bool is_grounded(btRigidBody* rigidbody, const btVector3& up,
        btScalar cosMaxSlope = btCos(btRadians(160.0f)),
        btScalar maxDist = btScalar(0.5))
    {
        const btTransform _transform = rigidbody->getWorldTransform();
        const btVector3 _origin = _transform.getOrigin();
        const btVector3 _from = _origin;
        const btVector3 _to = _origin - up.normalized() * (maxDist + 1.f);
        btCollisionWorld::ClosestRayResultCallback _raycast_callback(_from, _to);
        _raycast_callback.m_collisionFilterMask = detail::bulletgroupID_collider_world;
        detail::_dynamics_world->rayTest(_from, _to, _raycast_callback);
        if (_raycast_callback.hasHit()) {
            const btVector3 _hit_normal = _raycast_callback.m_hitNormalWorld.normalized();
            if (_hit_normal.dot(up) > cosMaxSlope) {
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
        btVector3 _hitpoint = _raycallback.m_hitPointWorld;
        btVector3 _hitnormal = _raycallback.m_hitNormalWorld.normalized();
#if LUCARIA_GUIZMO
        detail::_draw_guizmo_line(_hitpoint, _hitpoint + _hitnormal * 0.2f, btVector3(1, 1, 1)); // white
#endif
        return raycast_collision { detail::convert(_hitpoint), detail::convert(_hitnormal) };
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
            scene.view<collider_component>(entt::exclude<transform_component>).each([](collider_component& _collider) {
                _collider._shape.has_value(); // colliders can have no transform
            });
            scene.view<collider_component, transform_component>().each([](collider_component& _collider, transform_component& _transform) {
                if (!_collider._shape.has_value()) {
                    return;
                }
                const btTransform _bullet_transform = convert_bullet(_transform._transform);
                _collider._rigidbody->setWorldTransform(_bullet_transform);
            });
        });

        // update kinematic rigidbody transforms
        detail::each_scene([&](entt::registry& scene) {
            scene.view<kinematic_rigidbody_component, transform_component>().each([](kinematic_rigidbody_component& _rigidbody, transform_component& _transform) {
                if (!_rigidbody._shape.has_value()) {
                    return;
                }
                const btTransform _bullet_transform = convert_bullet(_transform._transform);
                _rigidbody._ghost->setWorldTransform(_bullet_transform);
            });
        });

        // apply dynamic rigidbody forces
        detail::each_scene([&](entt::registry& scene) {
            scene.view<dynamic_rigidbody_component>().each([&](dynamic_rigidbody_component& _rigidbody) {
                if (!_rigidbody._shape.has_value()) {
                    return;
                }
                btRigidBody* _bullet_rigidbody = _rigidbody._rigidbody.get();
                const btTransform _bullet_transform = _bullet_rigidbody->getWorldTransform();
                const btVector3 _bullet_origin = _bullet_transform.getOrigin();
                const btQuaternion _bullet_rotation = _bullet_transform.getRotation();
                const btVector3 _bullet_linear_velocity = _bullet_rigidbody->getLinearVelocity();
                const btVector3 _bullet_angular_velocity = _bullet_rigidbody->getAngularVelocity();
                const btVector3 _bullet_up = btVector3(0, 1, 0);
                const btVector3 v_d = detail::convert_bullet(_rigidbody._target_linear_velocity);
                const btVector3 p_d = detail::convert_bullet(_rigidbody._target_linear_position);
                const btQuaternion q_d = detail::convert_bullet(_rigidbody._target_angular_position);
                const btVector3 w_d = detail::convert_bullet(_rigidbody._target_angular_velocity);

                // linear PD
                const btVector3 e_p_xy = project_on_plane_bullet(p_d - _bullet_origin, _bullet_up);
                const btVector3 e_v_xy = project_on_plane_bullet(v_d - _bullet_linear_velocity, _bullet_up);
                btVector3 F = btScalar(_rigidbody._linear_kp) * e_p_xy + btScalar(_rigidbody._linear_kd) * e_v_xy;
                const btScalar Fmax = btScalar(_rigidbody._linear_max_force);
                const btScalar Flen2 = F.length2();
                if (Flen2 > Fmax * Fmax) {
                    F *= Fmax / btSqrt(Flen2);
                }
                _rigidbody._linear_forces += convert(F);

                // angular PD
                auto forwardXY = [&](const btQuaternion& qq) {
                    btVector3 f = quatRotate(qq, btVector3(0, 0, 1));
                    f = project_on_plane_bullet(f, _bullet_up);
                    btScalar L = f.length();
                    return (L > 0.f) ? (f * (1.f / L)) : btVector3(0, 0, 1);
                };
                const btVector3 f_now = forwardXY(_bullet_rotation);
                const btVector3 f_des = forwardXY(q_d);
                const btScalar cosang = btClamped(f_now.dot(f_des), btScalar(-1), btScalar(1));
                const btScalar sinang = (f_now.cross(f_des)).dot(_bullet_up);
                const btScalar yaw_err = btAtan2(sinang, cosang); // radians
                const btScalar yaw_rate_d = w_d.dot(_bullet_up);
                const btScalar yaw_rate = _bullet_angular_velocity.dot(_bullet_up);
                const bool grounded = is_grounded(_bullet_rigidbody, _bullet_up);
                const btScalar KpR = btScalar(_rigidbody._angular_kp) * (grounded ? btScalar(1) : btScalar(_rigidbody._angular_airborne_scale));
                const btScalar KdR = btScalar(_rigidbody._angular_kd) * (grounded ? btScalar(1) : btScalar(_rigidbody._angular_airborne_scale));
                btVector3 Tau = _bullet_up * (KpR * yaw_err + KdR * (yaw_rate_d - yaw_rate));
                const btScalar Tmax = btScalar(_rigidbody._angular_max_force);
                const btScalar Tlen = Tau.length();
                if (Tlen > Tmax && Tlen > 0.f) {
                    Tau *= Tmax / Tlen;
                }
                _rigidbody._angular_forces += convert(Tau);

                // apply
                _bullet_rigidbody->applyCentralForce(convert_bullet(_rigidbody._linear_forces));
                _bullet_rigidbody->applyTorque(convert_bullet(_rigidbody._angular_forces));
                _bullet_rigidbody->applyCentralImpulse(convert_bullet(_rigidbody._linear_impulses));
                _bullet_rigidbody->applyTorqueImpulse(convert_bullet(_rigidbody._angular_impulses));
                _bullet_rigidbody->activate(true);
                _rigidbody._linear_forces = glm::vec3(0);
                _rigidbody._angular_forces = glm::vec3(0);
                _rigidbody._linear_impulses = glm::vec3(0);
                _rigidbody._angular_impulses = glm::vec3(0);
            });
        });

        detail::_dynamics_world->stepSimulation(static_cast<float>(get_time_delta()), 10);
    }

    void dynamics_system::compute_collisions()
    {
        // kinematic
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
                        btPersistentManifold* _manifold;
                        btCollisionObject* _other_object;
                        if (!get_manifold(&_manifold, _manifold_array, _manifold_index)) {
                            continue;
                        }
                        if (!get_other_object(&_other_object, _manifold, rigidbody._ghost.get())) {
                            continue;
                        }
                        if (!get_other_group(_other_group, _other_object)) {
                            continue;
                        }
                        if (_other_group == bulletgroupID_collider_world) {
                            rigidbody._world_collisions = get_collisions(_manifold, rigidbody._ghost.get());
                        } else {
                            rigidbody._layer_collisions[static_cast<kinematic_layer>(_other_group)] = get_collisions(_manifold, rigidbody._ghost.get());
                        }
                    }
                }
                rigidbody._translation_speed = convert(rigidbody._ghost->getInterpolationLinearVelocity());
                rigidbody._rotation_speed = convert(rigidbody._ghost->getInterpolationAngularVelocity());
            });
        });

        // dynamic
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
