#include <ecs/component/collider.hpp>

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>::collider_component(collider_component&& other)
{
    *this = std::move(other);
}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>& collider_component<algorithm_t>::operator=(collider_component&& other)
{
    return *this;
}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>::~collider_component()
{

}

template <collider_algorithm algorithm_t>
collider_component<algorithm_t>& collider_component<algorithm_t>::navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh)
{
    _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    _lol.emplace(fetched_navmesh, [this] () {
        btCollisionShape* _shape = _lol.value().get_shape();
        btRigidBody::btRigidBodyConstructionInfo _construction(0, _state, _shape, btVector3(0, 0, 0));
        _rigidbody = new btRigidBody(_construction);

        // dynamicsWorld->addRigidBody(surfaceRigidBody);
    });
    
    return *this;
}
