#include <ecs/system/dynamics.hpp>

#include <btBulletDynamicsCommon.h>

namespace detail {

static bool is_bullet_setup = false;
static btDefaultCollisionConfiguration* collision_configuration = nullptr;
static btCollisionDispatcher* dispatcher = nullptr;
static btBroadphaseInterface* overlapping_pair_cache = nullptr;
static btSequentialImpulseConstraintSolver* solver = nullptr;
static btDiscreteDynamicsWorld* dynamics_world = nullptr;

}




void dynamics_system::gravity(const glm::vec3& newtons)
{
    detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

void dynamics_system::update()
{
    if (!detail::is_bullet_setup) {
        detail::collision_configuration = new btDefaultCollisionConfiguration();
        detail::dispatcher = new btCollisionDispatcher(detail::collision_configuration);
        detail::overlapping_pair_cache = new btDbvtBroadphase();
        detail::solver = new btSequentialImpulseConstraintSolver();
        detail::dynamics_world = new btDiscreteDynamicsWorld(detail::dispatcher, detail::overlapping_pair_cache, detail::solver, detail::collision_configuration);
        detail::dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
        detail::is_bullet_setup = true;
    }
}

void dynamics_system::prevent_kinematic_wall_collisions()
{
    // transforms, rigidbodies \\ colliders (from bullet)
}

void dynamics_system::snap_kinematic_grounds()
{
    // transforms, rigidbodies \\ colliders (from bullet)
}