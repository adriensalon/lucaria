#include <ecs/component/animator.hpp>
#include <core/fetch.hpp>



animator_component& animator_component::skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton)
{
    _fetched_skeleton = fetched_skeleton;
    _skeleton = nullptr;
    return *this;
}

animator_component& animator_component::play(const glm::uint& id)
{
    _moveset->get_animation(id).is_playing = true;
    return *this;
}