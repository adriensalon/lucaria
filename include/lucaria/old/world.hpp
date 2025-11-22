#pragma once

#include <functional>
#include <vector>

#include <entt/entt.hpp>

namespace lucaria {
namespace detail {

    inline std::vector<entt::registry>* global_scenes = nullptr;

    void set_scenes(std::vector<entt::registry>& scenes);
    void each_scene(const std::function<void(entt::registry&)>& callback);
    void destroy_scenes();
    [[nodiscard]] std::size_t get_scenes_count();

}
}