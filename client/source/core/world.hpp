#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

void register_level(const glm::uint id, const std::function<void(entt::registry&)>& callback);
void add_level(const glm::uint id);
void remove_level(const glm::uint id);
void each_level(const std::function<void(entt::registry&)>& callback);
