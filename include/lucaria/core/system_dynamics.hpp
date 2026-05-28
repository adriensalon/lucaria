#pragma once

#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/utils_collision.hpp>

namespace lucaria {
namespace detail {

	struct system_rendering;

    struct system_dynamics {
        system_dynamics();
        system_dynamics(const system_dynamics& other) = delete;
        system_dynamics& operator=(const system_dynamics& other) = delete;
        system_dynamics(system_dynamics&& other) = delete;
        system_dynamics& operator=(system_dynamics&& other) = delete;

        const float32x3 world_up = float32x3(0, 1, 0);
        const float32x3 world_forward = float32x3(0, 0, -1);
		btDiscreteDynamicsWorld* dynamics_world = nullptr;
        btDefaultCollisionConfiguration* collision_configuration = nullptr;
        btCollisionDispatcher* collision_dispatcher = nullptr;
        btBroadphaseInterface* overlapping_pair_cache = nullptr;
        btSequentialImpulseConstraintSolver* constraint_solver = nullptr;

        void update_step_simulation(manager_window& window, manager_scene& scenes);
        void update_compute_collisions(manager_scene& scenes);
        void update_collect_debug_guizmos(system_rendering& rendering, manager_scene& scenes);

		[[nodiscard]] std::optional<collision> raycast(system_rendering& rendering, const float32x3& from, const float32x3& to);
		void set_world_gravity(const float32 newtons);
    };

}

}
