#include <functional>
#include <string>
#include <unordered_map>
#include <iostream>

#include <core/world.hpp>

#include <ecs/component/controller.hpp>

namespace detail {

inline std::unordered_map<std::string, std::function<void()>> resolvers = {};

}

template <typename state_t>
void scripting_system::use_controller_state()
{
    const std::string _name = typeid(state_t).name();
    if (detail::resolvers.find(_name) == detail::resolvers.end()) {
        detail::resolvers[_name] = [] {
            each_level([] (entt::registry& registry) {
                registry.view<controller_component<state_t>>().each([] (controller_component<state_t>& controller) {
                    for (std::function<void(state_t&)>& _script : controller._scripts) {
                        _script(controller._state);
                    }
                    controller._state.update();
                });
            });
        };
    }
}