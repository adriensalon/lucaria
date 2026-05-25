#include <lucaria/core/window.hpp>
#include <lucaria/entity/dynamics.hpp>
#include <lucaria/entity/mixer.hpp>
#include <lucaria/entity/motion.hpp>
#include <lucaria/entity/rendering.hpp>
#include <lucaria/entity/scene.hpp>

PSP_MODULE_INFO("LUCARIA_GAME_NAME_HERE", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int main(int argc, char** argv)
{
    lucaria::detail::manager_window _window(

        []() {
		lucaria::detail::update_callbacks();
		
		lucaria::detail::system_rendering::apply_camera_rotation();
        lucaria::detail::system_motion::advance_controllers();
        lucaria::detail::system_motion::apply_animations();
        lucaria::detail::system_motion::apply_motion_tracks();
        lucaria::detail::system_dynamics::step_simulation();
        lucaria::detail::system_dynamics::compute_collisions();
        lucaria::detail::system_mixer::apply_speaker_transforms();
        lucaria::detail::system_mixer::apply_listener_transform();
        lucaria::detail::system_motion::collect_debug_guizmos();
        lucaria::detail::system_dynamics::collect_debug_guizmos();
        lucaria::detail::system_rendering::clear_screen();
    	lucaria::detail::system_rendering::compute_projection();
		lucaria::detail::system_rendering::compute_view_projection();
		lucaria::detail::system_rendering::draw_skybox();
		lucaria::detail::system_rendering::draw_blockout_meshes();
		lucaria::detail::system_rendering::draw_unlit_meshes();
		lucaria::detail::system_rendering::draw_unlit_skinned_meshes();
		lucaria::detail::system_rendering::draw_imgui_spatial_interfaces();
		lucaria::detail::system_rendering::draw_post_processing();
		lucaria::detail::system_rendering::draw_imgui_screen_interfaces();
		lucaria::detail::system_rendering::draw_debug_guizmos(); });
}