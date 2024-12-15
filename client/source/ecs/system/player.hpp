#pragma once

#include <future>

#include <ecs/component/transform.hpp>

struct player_system {
    player_system() = delete;
    player_system(const player_system& other) = delete;
    player_system& operator=(const player_system& other) = delete;
    player_system(player_system&& other) = delete;
    player_system& operator=(player_system&& other) = delete;

    static void player_position(const glm::vec3& position);
    static void player_direction(const glm::vec3& direction);
    // enlever au dessus et mettre 
    static void follow_transform(transform_component& value);
    static glm::mat4x4 get_view();
    
    static void update();


    static void use_transform(transform_component& transform);
};