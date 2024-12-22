#pragma once

#include <future>

#include <ecs/component/animator.hpp>
#include <ecs/component/transform.hpp>

struct player_system {
    player_system() = delete;
    player_system(const player_system& other) = delete;
    player_system& operator=(const player_system& other) = delete;
    player_system(player_system&& other) = delete;
    player_system& operator=(player_system&& other) = delete;

    static void follow_transform(transform_component& value);
    static void follow_bone(animator_component& value, const std::string& name);
    static glm::mat4x4 get_view();
    
    static void update();


    static void use_transform(transform_component& transform);
};