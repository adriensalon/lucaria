#pragma once

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::state(const state_t& new_state)
{
    _state = new_state;
    return *this;
}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::add_script(const std::function<void(state_t&)>& script)
{
    _scripts.emplace_back(script);
    return *this;
}