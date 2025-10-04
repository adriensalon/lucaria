#include <lucaria/core/run.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>

#include <lucaria/ecs/system/dynamics.hpp>
#include <lucaria/ecs/system/mixer.hpp>
#include <lucaria/ecs/system/motion.hpp>
#include <lucaria/ecs/system/rendering.hpp>

namespace lucaria {
void run(
    std::vector<entt::registry>& scenes,
    const std::function<void()>& on_start,
    const std::function<void()>& on_update)
{
#if defined(LUCARIA_DEBUG)
    std::cout << "[dev] running lucaria with OPTION debug ON" << std::endl;
#else
    std::cout << "[dev] running lucaria with OPTION debug OFF" << std::endl;
#endif
#if defined(LUCARIA_GUIZMO)
    std::cout << "[dev] running lucaria with OPTION guizmo ON" << std::endl;
#else
    std::cout << "[dev] running lucaria with OPTION guizmo OFF" << std::endl;
#endif

    detail::set_scenes(scenes);

    detail::run_game(on_start, [&]() {
        on_update();

        detail::motion_system::advance_controllers();
        detail::motion_system::apply_animations();
        detail::motion_system::apply_motion_tracks();

        detail::dynamics_system::step_simulation();
        detail::dynamics_system::compute_collisions();

        detail::mixer_system::apply_speaker_transforms();
        detail::mixer_system::apply_listener_transform();

        detail::rendering_system::compute_projection();

        detail::motion_system::collect_debug_guizmos();
        detail::dynamics_system::collect_debug_guizmos();

        detail::rendering_system::clear_screen();
        detail::rendering_system::compute_view_projection();
        detail::rendering_system::draw_skybox();
        detail::rendering_system::draw_blockout_meshes();
        detail::rendering_system::draw_unlit_meshes();
        detail::rendering_system::draw_debug_guizmos();
        detail::rendering_system::draw_imgui_spatial_interfaces();
        detail::rendering_system::draw_imgui_screen_interfaces();
        detail::rendering_system::clear_debug_guizmos(); });
}
}