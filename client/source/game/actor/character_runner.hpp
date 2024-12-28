#pragma once

#include <ecs/component/animator.hpp>
#include <ecs/component/transform.hpp>

struct character_runner_actor {
    character_runner_actor(scene_data& scene);
    void update();

    animator_component& get_animator();
    transform_component& get_transform();

private:
    std::optional<std::reference_wrapper<animator_component>> _animator = std::nullopt;
    std::optional<std::reference_wrapper<transform_component>> _transform = std::nullopt;
};