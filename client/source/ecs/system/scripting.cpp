#include <ecs/system/scripting.hpp>

void scripting_system::resolve_controller_states()
{
    for (std::pair<const std::string, std::function<void()>>& _pair : detail::resolvers) {
        _pair.second();
    }
}