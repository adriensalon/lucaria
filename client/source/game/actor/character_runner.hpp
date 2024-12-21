#pragma once

#include <core/world.hpp>
#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>

struct character_runner_actor {
    character_runner_actor(scene_data& scene);
    animator_component& get_animator();
    void update();

private:
    std::optional<std::reference_wrapper<animator_component>> _animator = std::nullopt;
    std::optional<std::reference_wrapper<transform_component>> _transform = std::nullopt;
};