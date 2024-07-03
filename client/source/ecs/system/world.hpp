#pragma once

#include <chrono>
#include <future>

#include <entt/entt.hpp>

#include <core/texture.hpp>

/// @brief 
struct world_system {

    /// @brief 
    world_system() = delete;

    /// @brief 
    /// @param other 
    world_system(const world_system& other) = delete;

    /// @brief 
    /// @param other 
    /// @return 
    world_system& operator=(const world_system& other) = delete;
    
    /// @brief 
    /// @param other 
    world_system(world_system&& other) = delete;

    /// @brief 
    /// @param other 
    /// @return 
    world_system& operator=(world_system&& other) = delete;

    /// @brief 
    static void update();

    /// @brief 
    /// @param name 
    /// @param callback 
    static void register_level(const std::string& name, const std::function<void(entt::registry&)>& callback);

    /// @brief 
    /// @param name 
    static void add_level(const std::string& name);

    /// @brief 
    /// @param name 
    static void remove_level(const std::string& name);

    /// @brief
    /// @param callback
    static void for_each(const std::function<void(entt::registry&)>& callback);

};