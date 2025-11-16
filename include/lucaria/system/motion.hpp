#pragma once

namespace lucaria {
namespace detail {
    
    struct motion_system {
        motion_system() = delete;
        motion_system(const motion_system& other) = delete;
        motion_system& operator=(const motion_system& other) = delete;
        motion_system(motion_system&& other) = delete;
        motion_system& operator=(motion_system&& other) = delete;

        static void advance_controllers();
        static void apply_animations(); // can be executed in parallel
        static void apply_motion_tracks(); // can be executed in parallel
        static void collect_debug_guizmos();
    };
}
}
