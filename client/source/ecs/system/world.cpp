#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include <ecs/system/world.hpp>

namespace detail {

std::unordered_map<std::string, std::pair<entt::registry, std::function<void(entt::registry&)>>> levels = {};

}

void world_system::update()
{

}

void world_system::register_level(const std::string& name, const std::function<void(entt::registry&)>& callback)
{
    detail::levels[name].second = callback;
}

void world_system::add_level(const std::string& name)
{
#if LUCARIA_DEBUG
    if (!detail::levels[name].second) {
        std::cout << "Level has not beed registered with world_system::register_level() yet" << std::endl;
        std::terminate();
    }
#endif
    std::pair<entt::registry, std::function<void(entt::registry&)>>& _pair = detail::levels.at(name);
    _pair.second(_pair.first);
}

void world_system::remove_level(const std::string& name)
{
#if LUCARIA_DEBUG
    if (!detail::levels[name].second) {
        std::cout << "Level has not beed registered with world_system::register_level() yet" << std::endl;
        std::terminate();
    }
#endif
    detail::levels.at(name).first.clear();
}

void world_system::for_each(const std::function<void(entt::registry&)>& callback)
{
    for (std::pair<const std::string, std::pair<entt::registry, std::function<void(entt::registry&)>>>& _pair : detail::levels) {
        callback(_pair.second.first);
    }
}
