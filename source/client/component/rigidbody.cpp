#include <btBulletDynamicsCommon.h>

#include <lucaria/component/rigidbody.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* _dynamics_world;

}

rigidbody_component<rigidbody_type::passive>::~rigidbody_component<rigidbody_type::passive>()
{
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
    }
}

rigidbody_component<rigidbody_type::passive>& rigidbody_component<rigidbody_type::passive>::use_shape(shape& from)
{
    _shape.emplace(from);
    btCollisionShape* _collision_shape = _shape.value().get_handle();
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        _rigidbody->setCollisionShape(_collision_shape);
    } else {
        _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1)));
        _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0, _state.get(), _collision_shape));
        _rigidbody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
    }
    detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    _is_added = true;
    return *this;
}

rigidbody_component<rigidbody_type::passive>& rigidbody_component<rigidbody_type::passive>::use_shape(fetched<shape>& from)
{
    _shape.emplace(from, [this]() {
        btCollisionShape* _collision_shape = _shape.value().get_handle();
        if (_is_added) {
            detail::_dynamics_world->removeRigidBody(_rigidbody.get());
            _rigidbody->setCollisionShape(_collision_shape);
        } else {
            _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1)));
            _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0, _state.get(), _collision_shape));
            _rigidbody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
        }
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        _is_added = true;
    });
    return *this;
}

rigidbody_component<rigidbody_type::passive>& rigidbody_component<rigidbody_type::passive>::set_group_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _group |= _layer_bitfield;
    } else {
        _group &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_type::passive>& rigidbody_component<rigidbody_type::passive>::set_mask_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _mask |= _layer_bitfield;
    } else {
        _mask &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_type::kinematic>& rigidbody_component<rigidbody_type::kinematic>::use_shape(shape& from)
{
    _shape.emplace(from);
    btCollisionShape* _collision_shape = _shape.value().get_handle();
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        _ghost->setCollisionShape(_collision_shape);
    } else {
        _ghost = std::make_unique<btPairCachingGhostObject>();
        _ghost->setCollisionShape(_collision_shape);
        _ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    _is_added = true;
    return *this;
}

rigidbody_component<rigidbody_type::kinematic>& rigidbody_component<rigidbody_type::kinematic>::use_shape(fetched<shape>& from)
{
    _shape.emplace(from, [this]() {
        btCollisionShape* _collision_shape = _shape.value().get_handle();
        if (_is_added) {
            detail::_dynamics_world->removeCollisionObject(_ghost.get());
            _ghost->setCollisionShape(_collision_shape);
        } else {
            _ghost = std::make_unique<btPairCachingGhostObject>();
            _ghost->setCollisionShape(_collision_shape);
            _ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
        }
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
        _is_added = true;
    });
    return *this;
}

rigidbody_component<rigidbody_type::kinematic>& rigidbody_component<rigidbody_type::kinematic>::set_group_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _group |= _layer_bitfield;
    } else {
        _group &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_type::kinematic>& rigidbody_component<rigidbody_type::kinematic>::set_mask_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _mask |= _layer_bitfield;
    } else {
        _mask &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeCollisionObject(_ghost.get());
        detail::_dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
    }
    return *this;
}

const std::vector<kinematic_collision>& rigidbody_component<rigidbody_type::kinematic>::get_collisions() const
{
    return _collisions;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::use_shape(shape& from)
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
        _rigidbody->setCollisionFlags(btCollisionObject::CF_DYNAMIC_OBJECT);
    }
    detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    _is_added = true;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::use_shape(fetched<shape>& from)
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
            _rigidbody->setCollisionFlags(btCollisionObject::CF_DYNAMIC_OBJECT);
        }
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        _is_added = true;
    });
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_group_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _group |= _layer_bitfield;
    } else {
        _group &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_mask_layer(const collision_layer layer, const bool enable)
{
    const std::int16_t _layer_bitfield = static_cast<std::int16_t>(layer);
    if (enable) {
        _mask |= _layer_bitfield;
    } else {
        _mask &= ~_layer_bitfield;
    }
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
        detail::_dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
    }
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_mass(const glm::float32 mass)
{
    _mass = mass;
    if (_is_added) {
        _rigidbody->setMassProps(_mass, btVector3(0.f, 0.f, 0.f));
    }
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_friction(const glm::float32 friction)
{
    _friction = friction;
    if (_is_added) {
        _rigidbody->setFriction(_friction);
        _rigidbody->activate(true);
    }
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_lock_angular(const glm::bvec3 lock)
{
    _angular_factor = glm::vec3(lock);
    if (_is_added) {
        _rigidbody->setAngularFactor(detail::convert_bullet(_angular_factor));
        _rigidbody->activate(true);
    }
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_linear_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force)
{
    _linear_kp = kp;
    _linear_kd = kd;
    _linear_max_force = max_force;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::set_angular_pd(const glm::float32 kp, const glm::float32 kd, const glm::float32 max_force)
{
    _angular_kp = kp;
    _angular_kd = kd;
    _angular_max_force = max_force;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::add_linear_force(const glm::vec3& force)
{
    _linear_forces += force;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::add_angular_force(const glm::vec3& force)
{
    _angular_forces += force;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::add_linear_impulse(const glm::vec3& impulse)
{
    _linear_impulses += impulse;
    return *this;
}

rigidbody_component<rigidbody_type::dynamic>& rigidbody_component<rigidbody_type::dynamic>::add_angular_impulse(const glm::vec3& impulse)
{
    _angular_impulses += impulse;
    return *this;
}

glm::vec3 rigidbody_component<rigidbody_type::dynamic>::get_linear_speed()
{
    return _translation_speed;
}

glm::vec3 rigidbody_component<rigidbody_type::dynamic>::get_angular_speed()
{
    return _rotation_speed;
}

}
