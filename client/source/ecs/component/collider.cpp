#include <ecs/component/collider.hpp>
#include <ecs/system/dynamics.hpp>

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

// collider_component& collider_component::dynamic_layer()
// {
//     return *this;
// }

// collider_component& collider_component::kinematic_layer_wall()
// {
//     return *this;
// }

// collider_component& collider_component::kinematic_layer_ground()
// {
//     return *this;
// }

// collider_component& collider_component::kinematic_layer_game(const glm::uint layer)
// {
//     return *this;
// }

collider_component& collider_component::navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh)
{
    _navmesh.emplace(fetched_navmesh, [this]() {
        btCollisionShape* _shape = _navmesh.value().get_shape();
        _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1)));
        _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape));
        dynamics_system::get_dynamics_world()->addRigidBody(_rigidbody);
        _is_instanced = true;
    });
    return *this;
}
