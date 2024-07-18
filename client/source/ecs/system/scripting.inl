#include <functional>
#include <string>
#include <unordered_map>

#include <core/world.hpp>

#include <ecs/component/controller.hpp>

namespace detail {

inline std::unordered_map<std::string, std::function<void()>> resolvers = {};

}

template <typename state_t>
void scripting_system::use_controller_state()
{
    detail::resolvers[typeid(state_t).name()] = [] {
        each_level([] (entt::registry& registry) {
            registry.view<controller_component<state_t>>().each([] (controller_component<state_t>& controller) {
                if (controller._resolver) {
                    controller._resolver(controller._state);
                }
            });
        });
    };
}