#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

constexpr glm::uint levelID_persistent_player = 0;
constexpr glm::uint levelID_blockout_test = 1;

void level_persistent_player(entt::registry& registry);
void level_blockout_test(entt::registry& registry);