#pragma once

#include <future>

#include <core/volume.hpp>
#include <glue/update.hpp>

/// @brief
struct player_system {
    player_system() = delete;
    player_system(const player_system& other) = delete;
    player_system& operator=(const player_system& other) = delete;
    player_system(player_system&& other) = delete;
    player_system& operator=(player_system&& other) = delete;

    /// @brief
    /// @param position
    static void player_position(const glm::vec3& position);

    /// @brief
    /// @param direction
    static void player_direction(const glm::vec3& direction);

    /// @brief
    /// @param height
    static void player_height(const float height);

    /// @brief
    /// @param radius
    static void player_radius(const float radius);

    /// @brief
    /// @return
    static glm::mat4x4 get_view();
    
    static void update();

private:
    REGISTER_FOR_UPDATE(player_system)
};