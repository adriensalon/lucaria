#include <core/world.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/system/dynamics.hpp>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

static glm::mat4 bullet_to_glm(const btTransform& transform) {
    glm::mat4 matrix;

    // Extract rotation matrix (btMatrix3x3)
    const btMatrix3x3& basis = transform.getBasis();
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            matrix[c][r] = basis[r][c];  // Transpose and copy
        }
    }

    // Extract translation vector (btVector3)
    const btVector3& origin = transform.getOrigin();
    matrix[3][0] = origin.x();
    matrix[3][1] = origin.y();
    matrix[3][2] = origin.z();
    matrix[3][3] = 1.0f;  // Homogeneous coordinate

    return matrix;
}

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
    each_level([] (entt::registry& registry) {
        registry.view<transform_component, rigidbody_component>().each([] (transform_component& transform, rigidbody_component& rigidbody) {
            const btTransform _transform = detail::glm_to_bullet(transform._transform);

            // rigidbody._rigidbody->getMotionState()->setWorldTransform(_transform);
            // rigidbody._rigidbody->setWorldTransform(_transform);
            rigidbody._ghost->setWorldTransform(_transform);
        });
    });

    // detail::dynamics_world->stepSimulation(1.f  / 60.f);
    detail::dynamics_world->stepSimulation(1.f  / 30.f, 10);
    // detail::dynamics_world->getCollisionWorld()->
    // std::cout << detail::dynamics_world->getNumCollisionObjects() << std::endl;

    each_level([] (entt::registry& registry) {
        registry.view<transform_component, rigidbody_component>().each([] (transform_component& transform, rigidbody_component& rigidbody) {

            // rigidbody._rigidbody->getMotionState()->setWorldTransform(_transform);
            // rigidbody._rigidbody->setWorldTransform(_transform);
            // rigidbody._ghost->setWorldTransform(_transform);

            btManifoldArray ManifoldArray;
            btBroadphasePairArray& PairArray = rigidbody._ghost->getOverlappingPairCache()->getOverlappingPairArray();

            for (int i = 0; i < PairArray.size(); i++)
            {
                ManifoldArray.clear();

                btBroadphasePair* CollisionPair = detail::dynamics_world->getPairCache()->findPair(PairArray[i].m_pProxy0, PairArray[i].m_pProxy1);
                if (!CollisionPair) {
                    continue;
                }
                std::cout << "YAAAAAAAAAAAAA \n";
                if (CollisionPair->m_algorithm)
                {
                    CollisionPair->m_algorithm->getAllContactManifolds(ManifoldArray);
                }

                for (int j = 0; j < ManifoldArray.size(); j++)
                {
                    for (int p = 0; p < ManifoldArray[j]->getNumContacts(); p++)
                    {
                        const btManifoldPoint& Point = ManifoldArray[j]->getContactPoint(p);

                        if (Point.getDistance() < 0.0f)
                        {
                            // return true;
                        }
                    }
                }
            }



            // transform._transform = detail::bullet_to_glm(rigidbody._ghost->getWorldTransform());

            // int numOverlappingPairs = rigidbody._ghost->ge();
            // for (int i = 0; i < numOverlappingPairs; ++i) {
            //     btCollisionObject* otherObject = rigidbody._ghost->getOverlappingObject(i);
            //     std::cout << "Ghost object is overlapping with another object." << std::endl;
            // }

            btTransform ghostTransform = rigidbody._ghost->getWorldTransform();
            btVector3 ghostPos = ghostTransform.getOrigin();
            // std::cout << "Ghost object position: (" << ghostPos.getX() << ", " << ghostPos.getY() << ", " << ghostPos.getZ() << ")" << std::endl;


        });
    });


    // std::cout << "num collision objs : " << detail::dynamics_world->getNumCollisionObjects() << std::endl;
    // int numManifolds = detail::dynamics_world->getDispatcher()->getNumManifolds();
    // for (int i = 0; i < numManifolds; i++) {
    //     btPersistentManifold* contactManifold = detail::dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);
    //     btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
    //     btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());
    //     std::cout << "MANIFOLD \n";
    //     // Check if bulletRigidBody is involved in the collision
    //     // if (obA == bulletRigidBody || obB == bulletRigidBody) {
    //         int numContacts = contactManifold->getNumContacts();
    //         for (int j = 0; j < numContacts; j++) {
    //             btManifoldPoint& pt = contactManifold->getContactPoint(j);
    //             if (pt.getDistance() < 0.f) {
    //                 const btVector3& ptA = pt.getPositionWorldOnA();
    //                 const btVector3& ptB = pt.getPositionWorldOnB();
    //                 const btVector3& normalOnB = pt.m_normalWorldOnB;

    //                 // Print collision point and normal
    //                 std::cout << "Collision point A: " << ptA.getX() << ", " << ptA.getY() << ", " << ptA.getZ() << std::endl;
    //                 std::cout << "Collision point B: " << ptB.getX() << ", " << ptB.getY() << ", " << ptB.getZ() << std::endl;
    //                 std::cout << "Normal on B: " << normalOnB.getX() << ", " << normalOnB.getY() << ", " << normalOnB.getZ() << std::endl;
    //             }
    //         }
    //     // }
    // }

}

void dynamics_system::snap_kinematic_grounds()
{
    // transforms, rigidbodies \\ colliders (from bullet)
}