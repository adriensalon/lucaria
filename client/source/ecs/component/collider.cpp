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
    return *this;
}
