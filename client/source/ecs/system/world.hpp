#pragma once

#include <chrono>
#include <future>

#include <entt/entt.hpp>

#include <core/texture.hpp>

struct world_system {
    world_system() = delete;
    world_system(const world_system& other) = delete;
    world_system& operator=(const world_system& other) = delete;
    world_system(world_system&& other) = delete;
    world_system& operator=(world_system&& other) = delete;

    static void register_level(const std::string& name, const std::function<void(entt::registry&)>& callback);
    static void add_level(const std::string& name);
    static void remove_level(const std::string& name);
    static void each_level(const std::function<void(entt::registry&)>& callback);
};