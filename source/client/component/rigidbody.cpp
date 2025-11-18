#include <btBulletDynamicsCommon.h>

#include <lucaria/component/rigidbody.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* _dynamics_world;

}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::use_shape(shape& from)
{
    _shape.emplace(from);
    btCollisionShape* _collision_shape = _shape.value().get_handle();
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        _ghost->setCollisionShape(_collision_shape);
    } else {
        _ghost = std::make_unique<btPairCachingGhostObject>();
        _ghost->setCollisionShape(_collision_shape);
        _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    _is_added = true;
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::use_shape(fetched<shape>& from)
{
    _shape.emplace(from, [this]() {
        btCollisionShape* _collision_shape = _shape.value().get_handle();
        if (_is_added) {
            detail::_dynamics_world->removeCollisionObject(_ghost.get());
            _ghost->setCollisionShape(_collision_shape);
        } else {
            _ghost = std::make_unique<btPairCachingGhostObject>();
            _ghost->setCollisionShape(_collision_shape);
            _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        }
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
        _is_added = true;
    });
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::set_collide_world(const bool enable)
{
    if (enable) {
        _mask = _mask | detail::bulletgroupID_collider_world;
    } else if (contains_layer(_mask, detail::bulletgroupID_collider_world)) {
        _mask = remove_layer(_mask, detail::bulletgroupID_collider_world);
    }
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::set_collide_layer(const kinematic_layer layer, const bool enable)
{
    const std::int16_t _layer = static_cast<std::int16_t>(layer);
    if (enable) {
        _mask = _mask | _layer;
    } else if (contains_layer(_mask, _layer)) {
        _mask = remove_layer(_mask, _layer);
    }
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    }
    return *this;
}

const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_world_collisions() const
{
    return _world_collisions;
}

const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_layer_collisions(const kinematic_layer layer) const
{
    return _layer_collisions.at(layer);
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::use_shape(shape& from)
{
    _shape.emplace(from);
    btCollisionShape* _collision_shape = _shape.value().get_handle();
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        _rigidbody->setCollisionShape(_collision_shape);
    } else {
        _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
        _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(_mass, _state.get(), _collision_shape, btVector3(0, 0, 0)));
        _rigidbody->setActivationState(DISABLE_DEACTIVATION);
        _rigidbody->setFriction(_friction);
        _rigidbody->setAngularFactor(detail::convert_bullet(_angular_factor));
        _rigidbody->setCcdMotionThreshold(0);
        _rigidbody->setCcdSweptSphereRadius(0);
    }
    detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    _is_added = true;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::use_shape(fetched<shape>& from)
{
    _shape.emplace(from, [this]() {
        btCollisionShape* _collision_shape = _shape.value().get_handle();
        if (_is_added) {
            detail::_dynamics_world->removeRigidBody(_rigidbody.get());
            _rigidbody->setCollisionShape(_collision_shape);
        } else {
            _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
            _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(_mass, _state.get(), _collision_shape, btVector3(0, 0, 0)));
            _rigidbody->setActivationState(DISABLE_DEACTIVATION);
            _rigidbody->setFriction(_friction);
            _rigidbody->setAngularFactor(detail::convert_bullet(_angular_factor));
            _rigidbody->setCcdMotionThreshold(0);
            _rigidbody->setCcdSweptSphereRadius(0);
        }
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        _is_added = true;
    });
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_mass(const glm::float32 mass)
{
    _mass = mass;
    if (_is_added) {
        _rigidbody->setMassProps(_mass, btVector3(0.f, 0.f, 0.f));
    }
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_friction(const glm::float32 friction)
{
    _friction = friction;
    if (_is_added) {
        _rigidbody->setFriction(_friction);
        _rigidbody->activate(true);
    }
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_lock_angular(const glm::bvec3 lock)
{
    _angular_factor = glm::vec3(lock);
    if (_is_added) {
        _rigidbody->setAngularFactor(detail::convert_bullet(_angular_factor));
        _rigidbody->activate(true);
    }
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_linear_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force)
{
    _linear_kp = kp;
    _linear_kd = kd;
    _linear_max_force = max_force;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_angular_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force)
{
    _angular_kp = kp;
    _angular_kd = kd;
    _angular_max_force = max_force;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::add_linear_force(const glm::vec3& force)
{
    _linear_forces += force;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::add_angular_force(const glm::vec3& force)
{
    _angular_forces += force;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::add_linear_impulse(const glm::vec3& impulse)
{
    _linear_impulses += impulse;
    return *this;
}

rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::add_angular_impulse(const glm::vec3& impulse)
{
    _angular_impulses += impulse;
    return *this;
}

glm::vec3 rigidbody_component<rigidbody_kind::dynamic>::get_linear_speed()
{
    return _translation_speed;
}

glm::vec3 rigidbody_component<rigidbody_kind::dynamic>::get_angular_speed()
{
    return _rotation_speed;
}

}
