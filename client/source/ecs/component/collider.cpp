#include <ecs/component/collider.hpp>
#include <ecs/system/dynamics.hpp>

namespace detail {

extern btDiscreteDynamicsWorld* dynamics_world;

}

collider_component::collider_component(collider_component&& other)
{
    *this = std::move(other);
}

collider_component& collider_component::operator=(collider_component&& other)
{
    _navmesh = std::move(other._navmesh);
    _state = other._state;
    _rigidbody = other._rigidbody;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

collider_component::~collider_component()
{
    if (_is_instanced) {
        delete _rigidbody->getMotionState();
        delete _rigidbody;
    }
}

collider_component& collider_component::navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh)
{
    _navmesh.emplace(fetched_navmesh, [this]() {
        btCollisionShape* _shape = _navmesh.value().get_shape();
        _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
        _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape));
        detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
        _is_instanced = true;
    });
    return *this;
}

collider_component& collider_component::ground()
{
    if (_group != bulletgroupID_collider_ground) {
        _group = bulletgroupID_collider_ground;
        _mask = _mask | bulletgroupID_dynamic_rigidbody;
        if (_is_instanced) {
            detail::dynamics_world->removeRigidBody(_rigidbody);
            detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
        }
    }
    return *this;
}

collider_component& collider_component::wall()
{
    if (_group != bulletgroupID_collider_wall) {
        _group = bulletgroupID_collider_wall;
        _mask = _mask | bulletgroupID_dynamic_rigidbody;
        if (_is_instanced) {
            detail::dynamics_world->removeRigidBody(_rigidbody);
            detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
        }
    }
    return *this;
}

collider_component& collider_component::layer(const kinematic_layer layer)
{
    const short _layer = static_cast<short>(layer);
    if (_group != _layer) {
        _group = _layer;
        if (contains_layer(_mask, bulletgroupID_dynamic_rigidbody)) {
            _mask = remove_layer(_mask, bulletgroupID_dynamic_rigidbody);
        }
        if (_is_instanced) {
            detail::dynamics_world->removeRigidBody(_rigidbody);
            detail::dynamics_world->addRigidBody(_rigidbody, _group, _mask);
        }
    }
    return *this;
}
