#include <glm/gtc/constants.hpp>

#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/system/dynamics.hpp>

namespace detail {

extern btDiscreteDynamicsWorld* dynamics_world;

}

rigidbody_component<rigidbody_kind::kinematic>::rigidbody_component(rigidbody_component&& other)
{
    *this = std::move(other);
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::operator=(rigidbody_component&& other)
{
    _shape = other._shape;
    _ghost = other._ghost;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>::~rigidbody_component()
{
    if (_is_instanced) {
        delete _ghost;
        delete _shape;
    }
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::box(const glm::vec3& half_extents)
{
    _shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.z));
    _ghost = new btPairCachingGhostObject();
    _ghost->setCollisionShape(_shape);
    _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    detail::dynamics_world->addCollisionObject(_ghost, _group, _mask);
    _half_height = half_extents.y;
    _is_instanced = true;
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::capsule(const glm::float32 radius, const glm::float32 height)
{
    _shape = new btCapsuleShape(radius, height);
    _ghost = new btPairCachingGhostObject();
    _ghost->setCollisionShape(_shape);
    _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    detail::dynamics_world->addCollisionObject(_ghost, _group, _mask);
    _half_height = height / 2.f;
    _is_instanced = true;
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::collide_grounds(const bool enabled)
{
    _is_snap_ground = enabled;
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::collide_walls(const bool enabled)
{
    if (enabled) {
        _mask = _mask | bulletgroupID_collider_wall;
    } else if (contains_layer(_mask, bulletgroupID_collider_wall)) {
        _mask = remove_layer(_mask, bulletgroupID_collider_wall);
    }
    if (_is_instanced) {
        detail::dynamics_world->removeCollisionObject(_ghost);
        detail::dynamics_world->addCollisionObject(_ghost, _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::collide_layer(const kinematic_layer layer, const bool enabled)
{
    const short _layer = static_cast<short>(layer);
    if (enabled) {
        _mask = _mask | _layer;
    } else if (contains_layer(_mask, _layer)) {
        _mask = remove_layer(_mask, _layer);
    }
    if (_is_instanced) {
        detail::dynamics_world->removeCollisionObject(_ghost);
        detail::dynamics_world->addCollisionObject(_ghost, _group, _mask);
    }
    return *this;
}

const std::optional<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_ground_collision() const
{
    return _ground_collision;
}

const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_wall_collisions() const
{
    return _wall_collisions;
}

const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_layer_collisions(const kinematic_layer layer) const
{
    return _layer_collisions.at(layer);
}

rigidbody_component<rigidbody_kind::dynamic>::rigidbody_component(rigidbody_component&& other)
{
    *this = std::move(other);
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::operator=(rigidbody_component&& other)
{
    _shape = other._shape;
    _state = other._state;
    _rigidbody = other._rigidbody;
    _mass = other._mass;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>::~rigidbody_component()
{
    if (_is_instanced) {
        delete _rigidbody->getMotionState();
        delete _rigidbody;
        delete _shape;
    }
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::mass(const glm::float32 kilograms)
{
    _mass = kilograms;
    if (_is_instanced) {
        _rigidbody->setMassProps(_mass, btVector3(0.f, 0.f, 0.f));
    }
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::box(const glm::vec3& half_extents)
{
    _shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.z));
    _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(_mass, _state, _shape, btVector3(0, 0, 0)));
    detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
    _is_instanced = true;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::capsule(const glm::float32 radius, const glm::float32 height)
{
    _shape = new btCapsuleShape(radius, height);
    _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(_mass, _state, _shape, btVector3(0, 0, 0)));
    detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
    _is_instanced = true;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::collide_dynamics(const bool enabled)
{
    if (enabled) {
        _mask = _mask | bulletgroupID_dynamic_rigidbody;
    } else if (contains_layer(_mask, bulletgroupID_dynamic_rigidbody)) {
        _mask = remove_layer(_mask, bulletgroupID_dynamic_rigidbody);
    }
    if (_is_instanced) {
        detail::dynamics_world->removeRigidBody(_rigidbody);
        detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
    }
    return *this;
}
