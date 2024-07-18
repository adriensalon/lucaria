#include <ecs/component/collider.hpp>

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>::collider_component(collider_component&& other)
{
    *this = std::move(other);
}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>& collider_component<algorithm_t>::operator=(collider_component&& other)
{
    _navmesh = std::move(other._navmesh);
    _state = other._state;
    _rigidbody = other._rigidbody;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>::~collider_component()
{
    if (_is_instanced) {
        delete _rigidbody->getMotionState();
        delete _rigidbody;
    }
}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>& collider_component<algorithm_t>::navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh)
{
    _navmesh.emplace(fetched_navmesh, [this]() {
        btCollisionShape* _shape = _navmesh.value().get_shape();
        _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
        _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape, btVector3(0, 0, 0)));

        // TODO REGISTER TO APPROPRIATE DYNAMICS WORLD FROM THE SYSTEM
        // dynamicsWorld->addRigidBody(surfaceRigidBody);
        _is_instanced = true;
    });
    return *this;
}

template struct collider_component<collider_algorithm::ground>;
template struct collider_component<collider_algorithm::wall>;