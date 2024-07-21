#include <ecs/component/animator.hpp>
#include <core/fetch.hpp>

animator_component& animator_component::animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations)
{
    for (const std::pair<const glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& _pair : fetched_animations) {
        _animations.emplace(_pair.first, fetch_container(_pair.second));
    }
    return *this;
}

animator_component& animator_component::armature(const std::shared_future<std::shared_ptr<armature_ref>>& fetched_armature)
{
    _armature.emplace(fetched_armature);
    return *this;
}

animator_component& animator_component::skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton)
{
    _skeleton.emplace(fetched_skeleton);
    return *this;
}

// animator_component& animator_component::moveset(const std::shared_future<std::shared_ptr<moveset_ref>>& fetched_moveset)
// {
//     _moveset.emplace(fetched_moveset);
//     return *this;
// }

// animator_component& animator_component::play(const glm::uint& id)
// {
//     _moveset.value().get_animation(id).is_playing = true;
//     return *this;
// }