#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/manager_app.hpp>

namespace lucaria {
namespace detail {

	struct system_rendering;

	struct system_motion {
        const float32x3 world_up = float32x3(0, 1, 0);
        const float32x3 world_forward = float32x3(0, 0, -1);

		void update_advance_controllers(manager_window& window, manager_scenes& scenes);
		void update_apply_animations(manager_scenes& scenes);
		void update_apply_motion_tracks(manager_window& window, manager_scenes& scenes);
		void update_collect_debug_guizmos(system_rendering& rendering, manager_scenes& scenes);
	};

}
}