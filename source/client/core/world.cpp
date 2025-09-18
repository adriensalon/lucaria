#include <lucaria/core/world.hpp>

void each_scene(const std::function<void(scene_data&)>& callback)
{
    // detail::scenes_registry.view<scene_data>().each([callback] (scene_data& scene) {
    //     callback(scene);
    //     // std::cout << "aaaaaaa \n";
    // });
    // std::cout << "\n";

    for (scene_data& _scene : detail::world_scenes) {
        callback(_scene);
    }
}


// #include <iostream>
// #include <string>
// #include <unordered_map>

// #include <core/world.hpp>

// namespace detail {

// std::vector<glm::uint> to_remove = {};
// std::unordered_map<glm::uint, std::pair<entt::registry, std::function<void(entt::registry&)>>> levels = {};
// std::unordered_set<glm::uint> loaded_levels = {};

// }

// void register_level(const glm::uint id, const std::function<void(entt::registry&)>& callback)
// {
//     detail::levels[id].second = callback;
// }

// void add_level(const glm::uint id)
// {
// #if LUCARIA_DEBUG
//     if (!detail::levels[id].second) {
//         std::cout << "Level has not beed registered with register_level() yet" << std::endl;
//         std::terminate();
//     }
// #endif
//     if (detail::loaded_levels.find(id) == detail::loaded_levels.end()) {
//         std::pair<entt::registry, std::function<void(entt::registry&)>>& _pair = detail::levels.at(id);
//         _pair.second(_pair.first);
//         detail::loaded_levels.emplace(id);
//     }
// }

// void mark_remove_level(const glm::uint id)
// {
// #if LUCARIA_DEBUG
//     if (!detail::levels[id].second) {
//         std::cout << "Level has not beed registered with register_level() yet" << std::endl;
//         std::terminate();
//     }
// #endif    
//     detail::to_remove.emplace_back(id);
//     detail::loaded_levels.erase(id);
// }

// void remove_levels()
// {
//     for (const glm::uint _id : detail::to_remove) {
//         detail::levels.at(_id).first.clear();
//         detail::levels.erase(_id);
//     }
//     detail::to_remove.clear();
// }

// void each_level(const std::function<void(entt::registry&)>& callback)
// {
//     for (std::pair<const glm::uint, std::pair<entt::registry, std::function<void(entt::registry&)>>>& _pair : detail::levels) {
//         callback(_pair.second.first);
//     }
// }
