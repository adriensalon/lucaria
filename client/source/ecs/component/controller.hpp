#pragma once

#include <functional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

template <typename state_t>
struct controller_component {
    controller_component() = default;
    controller_component(const controller_component& other) = delete;
    controller_component& operator=(const controller_component& other) = delete;
    controller_component(controller_component&& other) = default;
    controller_component& operator=(controller_component&& other) = default;

    controller_component& state(const state_t& new_state);
    controller_component& add_script(const std::function<void(state_t&)>& script);

private:
    state_t _state = {};
    std::function<void(state_t&)> _resolver = nullptr;
    std::vector<std::function<void(state_t&)>> _scripts = {};
    friend struct scripting_system;
};

#include <ecs/component/controller.inl>
