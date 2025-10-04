#pragma once

#include <functional>
#include <vector>

#include <entt/entt.hpp>

namespace lucaria {

/// @brief
/// @param scenes
/// @param on_start
/// @param on_update
void run(
    std::vector<entt::registry>& scenes,
    const std::function<void()>& on_start,
    const std::function<void()>& on_update);

}
