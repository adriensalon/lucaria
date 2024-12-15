#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

constexpr glm::uint levelID_menu_splash = 0;

constexpr glm::uint levelID_persistent_player = 11;
constexpr glm::uint levelID_blockout_test = 12;

constexpr glm::uint levelID_static_flight = 1000;

void level_menu_splash(entt::registry& registry);

void level_persistent_player(entt::registry& registry);
void level_blockout_test(entt::registry& registry);

void level_static_flight(entt::registry& registry);