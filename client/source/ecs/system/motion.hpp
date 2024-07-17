#pragma once

struct motion_system {
    motion_system() = delete;
    motion_system(const motion_system& other) = delete;
    motion_system& operator=(const motion_system& other) = delete;
    motion_system(motion_system&& other) = delete;
    motion_system& operator=(motion_system&& other) = delete;
    
    static void blend_animations();
    static void apply_root_motion();
    static void apply_foot_ik();
    static void skin_meshes();
};