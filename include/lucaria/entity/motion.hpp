#pragma once

namespace lucaria {
namespace detail {

	struct motion_system {
		static void advance_controllers();
		static void apply_animations();
		static void apply_motion_tracks();
		static void collect_debug_guizmos();
	};

}
}