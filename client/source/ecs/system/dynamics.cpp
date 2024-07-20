#include <core/hash.hpp>
#include <core/layer.hpp>
#include <core/mesh.hpp>
#include <core/program.hpp>
#include <core/window.hpp>
#include <core/world.hpp>
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

static float snap_ground_distance = 1.f;
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
        collision = kinematic_collision {
            glm::vec3(_point.getPositionWorldOnA().x(), _point.getPositionWorldOnA().y(), _point.getPositionWorldOnA().z()),
            glm::vec3(_point.m_normalWorldOnB.x(), _point.m_normalWorldOnB.y(), _point.m_normalWorldOnB.z()),
            _point.getDistance()
        };
        // std::cout << "collision at x = " << collision.impact_position.x
        //           << ", y = " << collision.impact_position.y
        //           << ", z = " << collision.impact_position.z << std::endl;
        // std::cout << "normal at x = " << collision.impact_normal.x
        //           << ", y = " << collision.impact_normal.y
        //           << ", z = " << collision.impact_normal.z << std::endl;
        return true;
    }
    return false;
}

static void compute_collide_wall(const kinematic_collision& collision, glm::mat4& transform)
{
    glm::vec3 _position = glm::vec3(transform[3]);
    glm::vec3 _normal_xz = glm::normalize(glm::vec3(collision.impact_normal.x, 0.f, collision.impact_normal.z));
    glm::vec3 _new_position = _position - _normal_xz * collision.penetration_distance;
    transform[3] = glm::vec4(_new_position, 1.0f);
}

static void compute_collide_ground(const kinematic_collision& collision, glm::mat4& transform)
{
    glm::vec3 _position = glm::vec3(transform[3]);
    glm::vec3 _new_position = _position - collision.impact_normal * collision.penetration_distance;
    transform[3] = glm::vec4(_new_position, 1.0f);
}

static void compute_snap_ground(glm::mat4& transform)
{
    
}

}

void dynamics_system::use_gravity(const glm::vec3& newtons)
{
    detail::dynamics_world->setGravity(btVector3(newtons.x, newtons.y, newtons.z));
}

void dynamics_system::use_snap_ground_distance(const float meters)
{
    detail::snap_ground_distance = meters;
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
    kinematic_collision _collision;
    btManifoldArray _manifold_array;
    each_level([&](entt::registry& registry) {
        registry.view<transform_component, kinematic_rigidbody_component>().each([&](transform_component& transform, kinematic_rigidbody_component& rigidbody) {
            rigidbody._ground_collisions.clear();
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
                    if (_other_group == bulletgroupID_collider_ground) {
                        detail::compute_collide_ground(_collision, transform._transform);
                        rigidbody._ground_collisions.emplace_back(_collision);
                    } else if (_other_group == bulletgroupID_collider_wall) {
                        detail::compute_collide_wall(_collision, transform._transform);
                        rigidbody._wall_collisions.emplace_back(_collision);
                    } else {
                        rigidbody._layer_collisions[static_cast<kinematic_layer>(_other_group)].emplace_back(_collision);
                    }
                }
            }
            detail::compute_snap_ground(transform._transform);
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
