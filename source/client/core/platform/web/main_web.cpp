#include <lucaria/core/window.hpp>
#include <lucaria/entity/dynamics.hpp>
#include <lucaria/entity/mixer.hpp>
#include <lucaria/entity/motion.hpp>
#include <lucaria/entity/rendering.hpp>
#include <lucaria/entity/scene.hpp>

int main(int argc, char** argv)
{
    lucaria::detail::window_implementation _window(

        []() {
		lucaria::detail::update_callbacks();
		
		lucaria::detail::rendering_system::apply_camera_rotation();
        lucaria::detail::motion_system::advance_controllers();
        lucaria::detail::motion_system::apply_animations();
        lucaria::detail::motion_system::apply_motion_tracks();
        lucaria::detail::dynamics_system::step_simulation();
        lucaria::detail::dynamics_system::compute_collisions();
        lucaria::detail::mixer_system::apply_speaker_transforms();
        lucaria::detail::mixer_system::apply_listener_transform();
        lucaria::detail::motion_system::collect_debug_guizmos();
        lucaria::detail::dynamics_system::collect_debug_guizmos();
        lucaria::detail::rendering_system::clear_screen();
    	lucaria::detail::rendering_system::compute_projection();
		lucaria::detail::rendering_system::compute_view_projection();
		lucaria::detail::rendering_system::draw_skybox();
		lucaria::detail::rendering_system::draw_blockout_meshes();
		lucaria::detail::rendering_system::draw_unlit_meshes();
		if (!lucaria::detail::engine_window().is_touch_supported) {
			lucaria::detail::rendering_system::draw_unlit_skinned_meshes();
		}
		lucaria::detail::rendering_system::draw_imgui_spatial_interfaces();
		lucaria::detail::rendering_system::draw_post_processing();
		lucaria::detail::rendering_system::draw_imgui_screen_interfaces();
		lucaria::detail::rendering_system::draw_debug_guizmos(); });
}