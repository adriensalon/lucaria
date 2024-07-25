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

    controller_component& state(state_t* state_ptr);

private:
    state_t* _state = nullptr;
    friend struct scripting_system;
};

#include <ecs/component/controller.inl>
