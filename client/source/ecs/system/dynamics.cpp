#include <ecs/system/dynamics.hpp>

#include <btBulletDynamicsCommon.h>

namespace detail {

static btDefaultCollisionConfiguration* collision_configuration = nullptr;
static btCollisionDispatcher* dispatcher = nullptr;
static btBroadphaseInterface* overlapping_pair_cache = nullptr;
static btSequentialImpulseConstraintSolver* solver = nullptr;
static btDiscreteDynamicsWorld* dynamics_world = nullptr;

static bool setup_bullet_worlds()
{
    collision_configuration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collision_configuration);
    overlapping_pair_cache = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamics_world = new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);
    dynamics_world->setGravity(btVector3(0.f, -9.81f, 0.f));
    return true;
}

static bool is_bullet_worlds_setup = setup_bullet_worlds();

}

void dynamics_system::use_gravity(const glm::vec3& newtons)
{
    detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

btDiscreteDynamicsWorld* dynamics_system::get_dynamics_world()
{
    return detail::dynamics_world;
}

void dynamics_system::prevent_kinematic_wall_collisions()
{
    // transforms, rigidbodies \\ colliders (from bullet)
}

void dynamics_system::snap_kinematic_grounds()
{
    // transforms, rigidbodies \\ colliders (from bullet)
}