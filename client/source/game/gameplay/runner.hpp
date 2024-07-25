#pragma once

#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/controller.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>

struct runner_actor {
    runner_actor() = delete;
    runner_actor(const runner_actor& other) = delete;
    runner_actor& operator=(const runner_actor& other) = delete;
    runner_actor(runner_actor&& other) = default;
    runner_actor& operator=(runner_actor&& other) = default;

    runner_actor(entt::registry& registry);
    void update();

    transform_component& get_transform();
    void add_script(const std::function<void(runner_actor&)>& script);

private:
    entt::entity _entity = {};
    rigidbody_component<rigidbody_kind::kinematic>* _rigidbody = nullptr;
    model_component<model_shader::unlit>* _model = nullptr;
    controller_component<runner_actor>* _controller = nullptr;
    transform_component* _transform = nullptr;
    animator_component* _animator = nullptr;
    speaker_component* _speaker = nullptr;
    std::vector<std::function<void(runner_actor&)>> _scripts = {};
};