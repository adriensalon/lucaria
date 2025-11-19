#include <btBulletDynamicsCommon.h>

#include <lucaria/component/collider.hpp>
#include <lucaria/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* _dynamics_world;

}

collider_component::~collider_component()
{
    if (_is_added) {
        detail::_dynamics_world->removeRigidBody(_rigidbody.get());
    }
}

collider_component& collider_component::use_shape(shape& from)
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

collider_component& collider_component::use_shape(fetched<shape>& from)
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

collider_component& collider_component::set_group_layer(const collision_layer layer, const bool enable)
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

collider_component& collider_component::set_mask_layer(const collision_layer layer, const bool enable)
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

}
