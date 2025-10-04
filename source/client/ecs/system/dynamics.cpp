#include <btBulletDynamicsCommon.h>

#include <lucaria/core/math.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/collider.hpp>
#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/component/transform.hpp>
#include <lucaria/ecs/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    btDiscreteDynamicsWorld* dynamics_world = nullptr;

}

namespace {

    static glm::float32 snap_ground_distance = 10.f;
    static btDefaultCollisionConfiguration* collision_configuration = nullptr;
    static btCollisionDispatcher* dispatcher = nullptr;
    static btBroadphaseInterface* overlapping_pair_cache = nullptr;
    static btSequentialImpulseConstraintSolver* solver = nullptr;

    static bool setup_bullet_worlds()
    {
        collision_configuration = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collision_configuration);
        overlapping_pair_cache = new btDbvtBroadphase();
        overlapping_pair_cache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
        solver = new btSequentialImpulseConstraintSolver();
        detail::dynamics_world = new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);
        detail::dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
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

    static bool get_other_group(short& other_group, const btCollisionObject* other_object)
    {
        const btBroadphaseProxy* _proxy = other_object->getBroadphaseHandle();
        if (_proxy) {
            other_group = _proxy->m_collisionFilterGroup;
            return true;
        }
        return false;
    }

    static bool get_collision(ecs::kinematic_collision& collision, const btPersistentManifold* manifold)
    {
        for (int _p = 0; _p < manifold->getNumContacts(); ++_p) {
            const btManifoldPoint& _point = manifold->getContactPoint(_p);
            if (_point.getDistance() > 0.0f) {
                continue;
            }
            collision.distance = _point.getDistance();
            collision.position = glm::vec3(_point.getPositionWorldOnA().x(), _point.getPositionWorldOnA().y(), _point.getPositionWorldOnA().z());
            collision.normal = glm::vec3(_point.m_normalWorldOnB.x(), _point.m_normalWorldOnB.y(), _point.m_normalWorldOnB.z());
            return true;
        }
        return false;
    }

    static void compute_collide_wall(const ecs::kinematic_collision& collision, glm::mat4& transform)
    {
        glm::vec3 _position = glm::vec3(transform[3]);
        glm::vec3 _normal_xz = glm::normalize(glm::vec3(collision.normal.x, 0.f, collision.normal.z));
        glm::vec3 _new_position = _position - _normal_xz * collision.distance;
        transform[3] = glm::vec4(_new_position, 1.0f);
    }

    // project onto plane orthogonal to "up"
    static inline btVector3 projOnPlane(const btVector3& v, const btVector3& up)
    {
        return v - up * v.dot(up);
    }

    // simple grounded test via manifolds (cos(max slope) ≈ cos 50°)
    static bool is_grounded(btRigidBody* body, const btVector3& up, btDynamicsWorld* world,
        btScalar cosMaxSlope = btCos(btRadians(160.0f)), // 60° slope
        btScalar maxDist = btScalar(0.5))
    {
        btTransform tr = body->getWorldTransform();
        btVector3 origin = tr.getOrigin();

        btVector3 from = origin;
        btVector3 to = origin - up.normalized() * (maxDist + 1.0f); // ray length > maxDist

        btCollisionWorld::ClosestRayResultCallback rayCB(from, to);
        rayCB.m_collisionFilterMask = ~0; // collide with everything
        world->rayTest(from, to, rayCB);

        if (rayCB.hasHit()) {
            btVector3 hitNormal = rayCB.m_hitNormalWorld.normalized();
            if (hitNormal.dot(up) > cosMaxSlope) {
                return true;
            }
        }
        return false;
    }
}

namespace ecs {
    namespace dynamics {

        void use_gravity(const glm::vec3& newtons)
        {
            detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
        }

        void use_snap_ground_distance(const glm::float32 meters)
        {
            snap_ground_distance = meters;
        }
    }
}

namespace detail {

    void dynamics_system::step_simulation()
    {
        detail::each_scene([&](entt::registry& scene) {
            scene.view<ecs::collider_component>().each([](ecs::collider_component& collider) {
                collider._shape.has_value(); // we only collect fetching shapes
            });

            scene.view<ecs::transform_component, ecs::kinematic_rigidbody_component>().each([](ecs::transform_component& transform, ecs::kinematic_rigidbody_component& rigidbody) {
                if (rigidbody._shape.has_value()) {
                    const glm::mat4 _feet_to_center = rigidbody._shape.value().get_feet_to_center();
                    const btTransform _transform = convert_bullet(_feet_to_center * transform._transform);
                    rigidbody._ghost->setWorldTransform(_transform);
                }
            });

            scene.view<ecs::rigidbody_component<ecs::rigidbody_kind::character>>().each([&](ecs::rigidbody_component<ecs::rigidbody_kind::character>& ch) {
                btRigidBody* body = ch._rigidbody.get();
                if (!body)
                    return;

                // --- read current physics state ---
                const btTransform T = body->getWorldTransform();
                const btVector3 x = T.getOrigin();
                const btQuaternion q = T.getRotation();
                const btVector3 v = body->getLinearVelocity();
                const btVector3 w = body->getAngularVelocity();
                const btVector3 up = detail::reinterpret_bullet(ch._up);

                // desired (filled in apply_motion_tracks)
                const btVector3 p_d = detail::reinterpret_bullet(ch._p_d);
                const btVector3 v_d = detail::reinterpret_bullet(ch._v_d);
                const btQuaternion q_d(ch._q_d.x, ch._q_d.y, ch._q_d.z, ch._q_d.w);
                const btVector3 w_d = detail::reinterpret_bullet(ch._w_d);

                // --- grounded? ---
                const bool grounded = is_grounded(body, up, detail::dynamics_world);

                // --- linear PD on horizontal plane ---
                const btVector3 e_p_xy = projOnPlane(p_d - x, up);
                const btVector3 e_v_xy = projOnPlane(v_d - v, up);

                btVector3 F = btScalar(ch._Kp_xy) * e_p_xy + btScalar(ch._Kd_xy) * e_v_xy;
                const btScalar Fmax = btScalar(ch._Fmax_xy);
                const btScalar Flen2 = F.length2();
                if (Flen2 > Fmax * Fmax)
                    F *= Fmax / btSqrt(Flen2);
                body->applyCentralForce(F);

                // --- yaw-only rotation PD about "up" ---
                auto forwardXY = [&](const btQuaternion& qq) {
                    btVector3 f = quatRotate(qq, btVector3(0, 0, 1));
                    f = projOnPlane(f, up);
                    btScalar L = f.length();
                    return (L > BT_ZERO) ? (f * (1.f / L)) : btVector3(0, 0, 1);
                };
                const btVector3 f_now = forwardXY(q);
                const btVector3 f_des = forwardXY(q_d);

                // signed yaw angle from now -> des around "up"
                const btScalar cosang = btClamped(f_now.dot(f_des), btScalar(-1), btScalar(1));
                const btScalar sinang = (f_now.cross(f_des)).dot(up);
                const btScalar yaw_err = btAtan2(sinang, cosang); // radians

                // desired yaw rate (take up-component from w_d; you set it in apply_motion_tracks)
                const btScalar yaw_rate_d = w_d.dot(up);
                const btScalar yaw_rate = w.dot(up);

                const btScalar KpR = btScalar(ch._Kp_rot) * (grounded ? btScalar(1) : btScalar(ch._air_rot_scale));
                const btScalar KdR = btScalar(ch._Kd_rot) * (grounded ? btScalar(1) : btScalar(ch._air_rot_scale));

                btVector3 Tau = up * (KpR * yaw_err + KdR * (yaw_rate_d - yaw_rate));
                const btScalar Tmax = btScalar(ch._Tmax_rot);
                const btScalar Tlen = Tau.length();
                if (Tlen > Tmax && Tlen > btScalar(0))
                    Tau *= Tmax / Tlen;

                body->applyTorque(Tau);

                body->activate(true);
            });
        });

        detail::dynamics_world->stepSimulation(static_cast<float>(get_time_delta()), 10);
    }

    void dynamics_system::compute_collisions()
    {
        // kinematic
        ecs::kinematic_collision _collision;
        btManifoldArray _manifold_array;
        detail::each_scene([&](entt::registry& scene) {
            scene.view<ecs::transform_component, ecs::kinematic_rigidbody_component>().each([&](ecs::transform_component& transform, ecs::kinematic_rigidbody_component& rigidbody) {
                rigidbody._ground_collision.reset();
                rigidbody._wall_collisions.clear();
                rigidbody._layer_collisions.clear();
                btBroadphasePairArray& _pair_array = rigidbody._ghost->getOverlappingPairCache()->getOverlappingPairArray();

                for (int _i = 0; _i < _pair_array.size(); _i++) {
                    _manifold_array.clear();
                    btBroadphasePair* _collision_pair = detail::dynamics_world->getPairCache()->findPair(_pair_array[_i].m_pProxy0, _pair_array[_i].m_pProxy1);
                    if (!_collision_pair) {
                        continue;
                    }
                    btCollisionAlgorithm* _collision_algorithm = _collision_pair->m_algorithm;
                    if (!_collision_algorithm) {
                        continue;
                    }
                    _collision_algorithm->getAllContactManifolds(_manifold_array);
                    for (int _j = 0; _j < _manifold_array.size(); _j++) {
                        short _other_group;
                        btPersistentManifold* _manifold;
                        btCollisionObject* _other_object;
                        if (!get_manifold(&_manifold, _manifold_array, _j)) {
                            continue;
                        }
                        if (!get_other_object(&_other_object, _manifold, rigidbody._ghost.get())) {
                            continue;
                        }
                        if (!get_other_group(_other_group, _other_object)) {
                            continue;
                        }
                        if (!get_collision(_collision, _manifold)) {
                            continue;
                        }
                        if (_other_group == bulletgroupID_collider_wall) {
                            compute_collide_wall(_collision, transform._transform);
                            rigidbody._wall_collisions.emplace_back(_collision);
                        } else {
                            rigidbody._layer_collisions[static_cast<kinematic_layer>(_other_group)].emplace_back(_collision);
                        }
                    }
                }
            });
        });

        // dynamic
        detail::each_scene([&](entt::registry& scene) {
            scene.view<ecs::transform_component, ecs::dynamic_rigidbody_component>().each([](ecs::transform_component& transform, ecs::dynamic_rigidbody_component& rigidbody) {
                const glm::mat4 _transform = convert(rigidbody._rigidbody->getWorldTransform());
                const glm::mat4 _center_to_feet = rigidbody._shape.value().get_center_to_feet();
                transform._transform = _transform * _center_to_feet;
            });
        });

        // character
        detail::each_scene([&](entt::registry& scene) {
            scene.view<ecs::transform_component, ecs::character_rigidbody_component>().each([](ecs::transform_component& transform, ecs::character_rigidbody_component& rigidbody) {
                const glm::mat4 _transform = convert(rigidbody._rigidbody->getWorldTransform());
                const glm::mat4 _center_to_feet = rigidbody._shape.value().get_center_to_feet();
                transform._transform = _transform * _center_to_feet;
            });
        });
    }

    void dynamics_system::collect_debug_guizmos()
    {
#if LUCARIA_GUIZMO
        detail::dynamics_world->debugDrawWorld();
#endif
    }

}
}
