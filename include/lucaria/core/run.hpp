#pragma once

#include <functional>
#include <vector>

#include <entt/entt.hpp>

namespace lucaria {

/// @brief Runs a Lucaria application from user owned registries
/// @param scenes user owned registries holding the components
/// @param on_start callback to fire after Lucaria has initialized
/// @param on_update callback to fire on evey frame
void run(
    std::vector<entt::registry>& scenes,
    const std::function<void()>& on_start,
    const std::function<void()>& on_update);

}
