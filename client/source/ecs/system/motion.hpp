#pragma once

#include <glue/update.hpp>

/// @brief
struct motion_system {
    motion_system() = delete;
    motion_system(const motion_system& other) = delete;
    motion_system& operator=(const motion_system& other) = delete;
    motion_system(motion_system&& other) = delete;
    motion_system& operator=(motion_system&& other) = delete;
    
    static void update();

private:
    REGISTER_FOR_UPDATE(motion_system)
};