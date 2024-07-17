#pragma once

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::state(const state_t& new_state)
{
    _state = new_state;
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::resolve(const std::function<void(state_t&)>& resolver)
{
    _resolver = resolver;
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_script(const std::function<void(state_t&)>& script)
{
    _scripts.emplace_back(script);
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_key_down_script(const std::function<void(const std::string&, state_t&)>& script)
{
    _key_down_scripts.emplace_back(script);
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_key_up_script(const std::function<void(const std::string&, state_t&)>& script)
{
    _key_up_scripts.emplace_back(script);
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_mouse_move_script(const std::function<void(const glm::vec2&, state_t&)>& script)
{
    _mouse_move_scripts.emplace_back(script);
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_mouse_button_down_script(const std::function<void(const glm::uint, state_t&)>& script)
{
    _mouse_button_down_scripts.emplace_back(script);
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_mouse_button_up_script(const std::function<void(const glm::uint, state_t&)>& script)
{
    _mouse_button_up_scripts.emplace_back(script);
    return *this;
}
