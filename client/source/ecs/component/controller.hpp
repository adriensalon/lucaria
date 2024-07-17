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
    controller_component& resolve(const std::function<void(state_t&)>& resolver);
    controller_component& add_script(const std::function<void(state_t&)>& script);
    controller_component& add_key_down_script(const std::function<void(const std::string&, state_t&)>& script);
    controller_component& add_key_up_script(const std::function<void(const std::string&, state_t&)>& script);
    controller_component& add_mouse_move_script(const std::function<void(const glm::vec2&, state_t&)>& script);
    controller_component& add_mouse_button_down_script(const std::function<void(const glm::uint, state_t&)>& script);
    controller_component& add_mouse_button_up_script(const std::function<void(const glm::uint, state_t&)>& script);

private:
    state_t _state = {};
    std::function<void(state_t&)> _resolver = nullptr;
    std::vector<std::function<void(state_t&)>> _scripts = {};
    std::vector<std::function<void(const std::string&, state_t&)>> _key_down_scripts = {};
    std::vector<std::function<void(const std::string&, state_t&)>> _key_up_scripts = {};
    std::vector<std::function<void(const glm::vec2&, state_t&)>> _mouse_move_scripts = {};
    std::vector<std::function<void(const glm::uint, state_t&)>> _mouse_button_down_scripts = {};
    std::vector<std::function<void(const glm::uint, state_t&)>> _mouse_button_up_scripts = {};
    friend struct scripting_system;
};

#include <ecs/component/controller.inl>
