#include <lucaria/core/window.hpp>
#include <lucaria/entity/dynamics.hpp>
#include <lucaria/entity/mixer.hpp>
#include <lucaria/entity/motion.hpp>
#include <lucaria/entity/rendering.hpp>
#include <lucaria/entity/scene.hpp>

int main(int argc, char** argv)
{
    lucaria::detail::manager_window _window(

        []() {
		lucaria::detail::update_callbacks();
		
		lucaria::detail::system_rendering::apply_camera_rotation();
        lucaria::detail::system_motion::advance_controllers();
        lucaria::detail::system_motion::apply_animations();
        lucaria::detail::system_motion::apply_motion_tracks();
        lucaria::detail::dynamics_system::step_simulation();
        lucaria::detail::dynamics_system::compute_collisions();
        lucaria::detail::system_mixer::apply_speaker_transforms();
        lucaria::detail::system_mixer::apply_listener_transform();
        lucaria::detail::system_motion::collect_debug_guizmos();
        lucaria::detail::dynamics_system::collect_debug_guizmos();
        lucaria::detail::system_rendering::clear_screen();
    	lucaria::detail::system_rendering::compute_projection();
		lucaria::detail::system_rendering::compute_view_projection();
		lucaria::detail::system_rendering::draw_skybox();
		lucaria::detail::system_rendering::draw_blockout_meshes();
		lucaria::detail::system_rendering::draw_unlit_meshes();
		if (!lucaria::detail::engine_window().is_touch_supported) {
			lucaria::detail::system_rendering::draw_unlit_skinned_meshes();
		}
		lucaria::detail::system_rendering::draw_imgui_spatial_interfaces();
		lucaria::detail::system_rendering::draw_post_processing();
		lucaria::detail::system_rendering::draw_imgui_screen_interfaces();
		lucaria::detail::system_rendering::draw_debug_guizmos(); });
}