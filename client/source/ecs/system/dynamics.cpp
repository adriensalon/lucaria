#include <core/hash.hpp>
#include <core/layer.hpp>
#include <core/mesh.hpp>
#include <core/program.hpp>
#include <core/world.hpp>
#include <core/window.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/dynamics.hpp>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if LUCARIA_GUIZMO
class guizmo_debug_draw : public btIDebugDraw {
public:
    std::unordered_map<glm::vec3, std::vector<glm::vec3>, vec3_hash> positions = {};
    std::unordered_map<glm::vec3, std::vector<glm::uvec2>, vec3_hash> indices = {};

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        const glm::vec3 _color(color.x(), color.y(), color.z());
        std::vector<glm::vec3>& _positions = positions[_color];
        std::vector<glm::uvec2>& _indices = indices[_color];
        const glm::uint _from_index = _positions.size();
        const glm::uint _to_index = _from_index + 1;
        _positions.emplace_back(from.x(), from.y(), from.z());
        _positions.emplace_back(to.x(), to.y(), to.z());
        _indices.emplace_back(glm::uvec2(_from_index, _to_index));
    }

    virtual void reportErrorWarning(const char* warning) override
    {
        std::cout << "Bullet warning: " << warning << std::endl;
    }

    virtual void drawContactPoint(const btVector3& point_on_b, const btVector3& normal_on_b, btScalar distance, int lifetime, const btVector3& color) override
    {
        drawLine(point_on_b, point_on_b + normal_on_b * distance, color);
    }

    virtual void draw3dText(const btVector3& location, const char* text) override
    {
        std::cout << "Bullet 3D text: " << text << " at (" << location.x() << ", " << location.y() << ", " << location.z() << ")" << std::endl;
    }

    virtual void setDebugMode(int mode) override
    {
        _debug_mode = mode;
    }

    virtual int getDebugMode() const override
    {
        return _debug_mode;
    }

private:
    int _debug_mode = DBG_DrawWireframe;
};
#endif

namespace detail {

static btDefaultCollisionConfiguration* collision_configuration = nullptr;
static btCollisionDispatcher* dispatcher = nullptr;
static btBroadphaseInterface* overlapping_pair_cache = nullptr;
static btSequentialImpulseConstraintSolver* solver = nullptr;

#if LUCARIA_GUIZMO
static guizmo_debug_draw* guizmo_draw = nullptr;
std::unordered_map<glm::vec3, guizmo_mesh_ref, vec3_hash> guizmo_meshes = {};
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
#if LUCARIA_GUIZMO
    guizmo_draw = new guizmo_debug_draw();
    dynamics_world->setDebugDrawer(guizmo_draw);
#endif
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

static glm::vec3 getPositionFromMatrix(const glm::mat4& matrix) {
    // Extract the translation (position) from the matrix
    glm::vec3 position(matrix[3][0], matrix[3][1], matrix[3][2]);
    return position;
}

std::optional<short> getCollisionGroup(btCollisionObject* object) {
    if (object) {
        btBroadphaseProxy* proxy = object->getBroadphaseHandle();
        if (proxy) {
            return proxy->m_collisionFilterGroup;
        }
    }
    return std::nullopt;
}

}

void dynamics_system::use_gravity(const glm::vec3& newtons)
{
    detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

void dynamics_system::step_simulation()
{
    each_level([](entt::registry& registry) {
        registry.view<transform_component, kinematic_rigidbody_component>().each([](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
            const btTransform _transform = detail::glm_to_bullet(transform._transform);
            rigidbody._ghost->setWorldTransform(_transform);
        });
    });
    detail::dynamics_world->stepSimulation(get_time_delta(), 10);
    each_level([](entt::registry& registry) {
        registry.view<transform_component, dynamic_rigidbody_component>().each([](transform_component& transform, dynamic_rigidbody_component& rigidbody) {
            const glm::mat4 _transform = detail::bullet_to_glm(rigidbody._rigidbody->getWorldTransform());
            transform._transform = _transform;
        });
    });
}

void dynamics_system::compute_kinematic_collisions()
{
    each_level([](entt::registry& registry) {
        registry.view<transform_component, kinematic_rigidbody_component>().each([](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
            
            
            btManifoldArray ManifoldArray;
            btBroadphasePairArray& PairArray = rigidbody._ghost->getOverlappingPairCache()->getOverlappingPairArray();
            for (int i = 0; i < PairArray.size(); i++) {
                ManifoldArray.clear();

                btBroadphasePair* CollisionPair = detail::dynamics_world->getPairCache()->findPair(PairArray[i].m_pProxy0, PairArray[i].m_pProxy1);
                if (!CollisionPair) {
                    continue;
                }
                btCollisionAlgorithm* collisionAlgorithm = CollisionPair->m_algorithm;
                if (!collisionAlgorithm) {
                    continue;
                }
                std::cout << "YAAAAAAAAAAAAA \n";
                collisionAlgorithm->getAllContactManifolds(ManifoldArray);

                for (int j = 0; j < ManifoldArray.size(); j++) {
                    for (int p = 0; p < ManifoldArray[j]->getNumContacts(); p++) {
                        const btManifoldPoint& Point = ManifoldArray[j]->getContactPoint(p);

                        if (Point.getDistance() < 0.0f) {
                            // return true;
                        }
                    }
                }
            }

            for (int j = 0; j < ManifoldArray.size(); j++) {
                btPersistentManifold* manifold = ManifoldArray[j];
                if (!manifold) {
                    continue;
                }

                btCollisionObject* objA = const_cast<btCollisionObject*>(manifold->getBody0());
                btCollisionObject* objB = const_cast<btCollisionObject*>(manifold->getBody1());
                btCollisionObject* otherObject = nullptr;

                if (objA == rigidbody._ghost) {
                    otherObject = objB;
                } else if (objB == rigidbody._ghost) {
                    otherObject = objA;
                } else {
                    continue; // Neither object is the ghost object, skip this manifold
                }


                std::optional<short> _group_other = detail::getCollisionGroup(otherObject);
                if (!_group_other) {
                    continue;
                }


                if (_group_other.value() == bulletgroupID_collider_ground) {
                    std::cout << "ground detected !" << std::endl;

                } else if (_group_other.value() == bulletgroupID_collider_wall) {
                    std::cout << "wall detected !" << std::endl;
                } else {
                    for (int p = 0; p < manifold->getNumContacts(); p++) {
                        const btManifoldPoint& point = manifold->getContactPoint(p);

                        if (point.getDistance() < 0.0f) {  // Contact detected (penetration)
                            kinematic_collision collision;
                            
                            // Convert Bullet types to glm types
                            collision.impact_position = glm::vec3(point.getPositionWorldOnA().x(), point.getPositionWorldOnA().y(), point.getPositionWorldOnA().z());
                            collision.impact_normal = glm::vec3(point.m_normalWorldOnB.x(), point.m_normalWorldOnB.y(), point.m_normalWorldOnB.z());
                            
                            const glm::vec3 position = detail::getPositionFromMatrix(transform._transform);
                            std::cout << "collision at x = " << collision.impact_position.x << ", y = " << collision.impact_position.y << ", z = " << collision.impact_position.z << std::endl;
                            std::cout << "normal at x = " << collision.impact_normal.x << ", y = " << collision.impact_normal.y << ", z = " << collision.impact_normal.z << std::endl;
                            std::cout << "position at x = " << position.x << ", y = " << position.y << ", z = " << position.z << std::endl;
                            
                            break;
                        }
                    }
                }



            }
        });
    });
}

void dynamics_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    for (std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : detail::guizmo_draw->positions) {
        const glm::vec3 _color = _pair.first;
        std::vector<glm::vec3>& _positions = _pair.second;
        std::vector<glm::uvec2>& _indices = detail::guizmo_draw->indices.at(_color);
        _positions.clear();
        _indices.clear();
    }
    detail::dynamics_world->debugDrawWorld();
    for (const std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : detail::guizmo_draw->positions) {
        const glm::vec3 _color = _pair.first;
        const std::vector<glm::vec3>& _positions = _pair.second;
        const std::vector<glm::uvec2>& _indices = detail::guizmo_draw->indices.at(_color);
        if (detail::guizmo_meshes.find(_color) == detail::guizmo_meshes.end()) {
            detail::guizmo_meshes.emplace(_color, guizmo_mesh_ref(_positions, _indices));
        } else {
            detail::guizmo_meshes.at(_color).update(_positions, _indices);
        }
    }
#endif
}
