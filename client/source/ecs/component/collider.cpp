#include <ecs/component/collider.hpp>

template <collider_detection detection_t>
collider_component<detection_t>& collider_component<detection_t>::volume(const std::shared_future<std::shared_ptr<volume_ref>>& fetched_volume)
{
    _fetched_volume = fetched_volume;
    return *this;
}

template struct collider_component<collider_detection::passive>;
template struct collider_component<collider_detection::navigator>;