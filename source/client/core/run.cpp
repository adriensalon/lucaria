#include <lucaria/core/run.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/system/dynamics.hpp>
#include <lucaria/system/mixer.hpp>
#include <lucaria/system/motion.hpp>
#include <lucaria/system/rendering.hpp>

#define _STRINGIFY(x) #x
#define _TO_STRING(x) _STRINGIFY(x)

namespace lucaria {
void run(
    std::vector<entt::registry>& scenes,
    const std::function<void()>& on_start,
    const std::function<void()>& on_update)
{
    detail::set_scenes(scenes);
    detail::run_game(

        // start callback
        [&]() {
            on_start();

            std::cout << "Built engine with generator: " << _TO_STRING(LUCARIA_GENERATOR) << std::endl;
            std::cout << "Built engine with compiler: " << _TO_STRING(LUCARIA_COMPILER) << std::endl;
            std::cout << "Built engine with config: " << _TO_STRING(LUCARIA_CONFIG) << std::endl;
            std::cout << "Built engine with simd: " << _TO_STRING(LUCARIA_SIMD) << std::endl;
            std::cout << "Built engine with exceptions: " << (LUCARIA_DEBUG ? "ON (Select config other than Debug to disable)" : "OFF (Select Debug config to enable)") << std::endl;
            std::cout << "Built engine with guizmos: " << (LUCARIA_GUIZMO ? "ON (Select config other than Debug to disable)" : "OFF (Select Debug config to enable)") << std::endl;
            std::cout << "Running engine with multitouch: " << (get_is_multitouch_supported() ? "ON" : "OFF") << std::endl;
            std::cout << "Running engine with compression: " << (get_is_etc2_supported() ? "ETC2" : (get_is_s3tc_supported() ? "S3TC" : "NONE")) << std::endl;
        },

        // update callback
        [&]() {
            on_update();

            detail::motion_system::advance_controllers();
            detail::motion_system::apply_animations();
            detail::motion_system::apply_motion_tracks();

            detail::dynamics_system::step_simulation();
            detail::dynamics_system::compute_collisions();

            detail::mixer_system::apply_speaker_transforms();
            detail::mixer_system::apply_listener_transform();
            
            detail::motion_system::collect_debug_guizmos();
            detail::dynamics_system::collect_debug_guizmos();

            detail::rendering_system::clear_screen();
            detail::rendering_system::compute_projection();
            detail::rendering_system::compute_view_projection();
            detail::rendering_system::draw_skybox();
            detail::rendering_system::draw_blockout_meshes();
            detail::rendering_system::draw_unlit_meshes();
            detail::rendering_system::draw_imgui_spatial_interfaces();
            detail::rendering_system::draw_post_processing();
            detail::rendering_system::draw_imgui_screen_interfaces();
            detail::rendering_system::draw_debug_guizmos();
        });
}
}