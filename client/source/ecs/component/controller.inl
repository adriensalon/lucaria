#pragma once

#include <functional>
#include <unordered_map>

#include <entt/entt.hpp>

#include <core/world.hpp>

namespace detail {

inline std::unordered_map<std::string, std::function<void()>> resolvers = {};

}

template <typename state_t>
controller_component<state_t>& controller_component<state_t>::state(state_t* state_ptr)
{
    // go dans ctor
    _state = state_ptr;
    const std::string _name = typeid(state_t).name();
    if (detail::resolvers.find(_name) == detail::resolvers.end()) {
        detail::resolvers[_name] = [] {
            each_level([] (entt::registry& registry) {
                registry.view<controller_component<state_t>>().each([] (controller_component<state_t>& controller) {
                    if (controller._state) {
                        controller._state->update();
                    }
                });
            });
        };
    }
    return *this;
}
