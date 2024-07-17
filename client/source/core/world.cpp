#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include <core/world.hpp>

namespace detail {

std::unordered_map<glm::uint, std::pair<entt::registry, std::function<void(entt::registry&)>>> levels = {};

}

void register_level(const glm::uint id, const std::function<void(entt::registry&)>& callback)
{
    detail::levels[id].second = callback;
}

void add_level(const glm::uint id)
{
#if LUCARIA_DEBUG
    if (!detail::levels[id].second) {
        std::cout << "Level has not beed registered with register_level() yet" << std::endl;
        std::terminate();
    }
#endif
    std::pair<entt::registry, std::function<void(entt::registry&)>>& _pair = detail::levels.at(id);
    _pair.second(_pair.first);
}

void remove_level(const glm::uint id)
{
#if LUCARIA_DEBUG
    if (!detail::levels[id].second) {
        std::cout << "Level has not beed registered with register_level() yet" << std::endl;
        std::terminate();
    }
#endif
    detail::levels.at(id).first.clear();
    detail::levels.erase(id);
}

void each_level(const std::function<void(entt::registry&)>& callback)
{
    for (std::pair<const glm::uint, std::pair<entt::registry, std::function<void(entt::registry&)>>>& _pair : detail::levels) {
        callback(_pair.second.first);
    }
}
