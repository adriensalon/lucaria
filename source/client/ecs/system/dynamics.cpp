#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <lucaria/core/hash.hpp>
#include <lucaria/core/layer.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/component/transform.hpp>
#include <lucaria/ecs/system/dynamics.hpp>

namespace detail {

static glm::float32 snap_ground_distance = 10.f;
static btDefaultCollisionConfiguration* collision_configuration = nullptr;
static btCollisionDispatcher* dispatcher = nullptr;
static btBroadphaseInterface* overlapping_pair_cache = nullptr;
static btSequentialImpulseConstraintSolver* solver = nullptr;


#if LUCARIA_GUIZMO


#endif

btDiscreteDynamicsWorld* dynamics_world = nullptr;

static bool setup_bullet_worlds()
{
    collision_configuration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collision_configuration);
    overlapping_pair_cache = new btDbvtBroadphase();
    overlapping_pair_cache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    solver = new btSequentialImpulseConstraintSolver();
    dynamics_world = new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);
    dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
    return true;
}

static bool is_bullet_worlds_setup = setup_bullet_worlds();

static btTransform glm_to_bullet(const glm::mat4& matrix)
{
    btMatrix3x3 _basis;
    _basis.setFromOpenGLSubMatrix(glm::value_ptr(matrix));
    btVector3 _origin(matrix[3][0], matrix[3][1], matrix[3][2]);
    return btTransform(_basis, _origin);
}

static glm::mat4 bullet_to_glm(const btTransform& transform)
{
    glm::mat4 _matrix;
    const btMatrix3x3& _basis = transform.getBasis();
    for (glm::uint _r = 0; _r < 3; ++_r) {
        for (glm::uint _r = 0; _r < 3; ++_r) {
            _matrix[_r][_r] = _basis[_r][_r];
        }
    }
    const btVector3& origin = transform.getOrigin();
    _matrix[3][0] = origin.x();
    _matrix[3][1] = origin.y();
    _matrix[3][2] = origin.z();
    _matrix[3][3] = 1.f;
    return _matrix;
}

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

static bool get_collision(kinematic_collision& collision, const btPersistentManifold* manifold)
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

static void compute_collide_wall(const kinematic_collision& collision, glm::mat4& transform)
{
    glm::vec3 _position = glm::vec3(transform[3]);
    glm::vec3 _normal_xz = glm::normalize(glm::vec3(collision.normal.x, 0.f, collision.normal.z));
    glm::vec3 _new_position = _position - _normal_xz * collision.distance;
    transform[3] = glm::vec4(_new_position, 1.0f);
}

static bool compute_snap_ground(glm::mat4& transform, kinematic_collision& collision, const glm::float32 half_height)
{
    glm::vec3 position = glm::vec3(transform[3]);
    btVector3 start(position.x, position.y + half_height, position.z);
    btVector3 end(position.x, position.y - detail::snap_ground_distance - half_height, position.z);
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
    rayCallback.m_collisionFilterGroup = bulletgroupID_kinematic_rigidbody;
    rayCallback.m_collisionFilterMask = bulletgroupID_collider_ground;
    detail::dynamics_world->rayTest(start, end, rayCallback);
    if (rayCallback.hasHit()) {
        collision.distance = glm::distance(position, collision.position);
        collision.position = glm::vec3(rayCallback.m_hitPointWorld.x(), rayCallback.m_hitPointWorld.y(), rayCallback.m_hitPointWorld.z());
        collision.normal = glm::vec3(rayCallback.m_hitNormalWorld.x(), rayCallback.m_hitNormalWorld.y(), rayCallback.m_hitNormalWorld.z());
        transform[3][1] = collision.position.y;
        return true;
    }
    return false;
}

}

void dynamics_system::use_gravity(const glm::vec3& newtons)
{
    detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

void dynamics_system::use_snap_ground_distance(const glm::float32 meters)
{
    detail::snap_ground_distance = meters;
}

void dynamics_system::step_simulation()
{
    each_scene([&](scene_data& scene) {
        scene.components.view<transform_component, kinematic_rigidbody_component>().each([](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
            const btTransform _transform = detail::glm_to_bullet(glm::translate(glm::mat4(1.f), glm::vec3(0.f, rigidbody._half_height, 0.f)) * transform._transform);
            rigidbody._ghost->setWorldTransform(_transform);
        });
    });
    detail::dynamics_world->stepSimulation(static_cast<float>(get_time_delta()), 10);
    each_scene([&](scene_data& scene) {
        scene.components.view<transform_component, dynamic_rigidbody_component>().each([](transform_component& transform, dynamic_rigidbody_component& rigidbody) {
            const glm::mat4 _transform = detail::bullet_to_glm(rigidbody._rigidbody->getWorldTransform());
            transform._transform = _transform;
        });
    });
}

void dynamics_system::compute_kinematic_collisions()
{
    kinematic_collision _collision;
    btManifoldArray _manifold_array;
    each_scene([&](scene_data& scene) {
        scene.components.view<transform_component, kinematic_rigidbody_component>().each([&](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
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
                    if (!detail::get_manifold(&_manifold, _manifold_array, _j)) {
                        continue;
                    }
                    if (!detail::get_other_object(&_other_object, _manifold, rigidbody._ghost)) {
                        continue;
                    }
                    if (!detail::get_other_group(_other_group, _other_object)) {
                        continue;
                    }
                    if (!detail::get_collision(_collision, _manifold)) {
                        continue;
                    }
                    if (_other_group == bulletgroupID_collider_wall) {
                        detail::compute_collide_wall(_collision, transform._transform);
                        rigidbody._wall_collisions.emplace_back(_collision);
                    } else {
                        rigidbody._layer_collisions[static_cast<kinematic_layer>(_other_group)].emplace_back(_collision);
                    }
                }
            }
            if (rigidbody._is_snap_ground) {
                if (detail::compute_snap_ground(transform._transform, _collision, rigidbody._half_height)) {
                    rigidbody._ground_collision = _collision;
                }
            }
        });
    });
}

void dynamics_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    detail::dynamics_world->debugDrawWorld();
#endif
}
