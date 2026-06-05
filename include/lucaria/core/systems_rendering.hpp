#pragma once

#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/rendering_framebuffer.hpp>
#include <lucaria/core/rendering_program.hpp>
#include <lucaria/core/rendering_guizmos.hpp>
#include <lucaria/engine/asset_cubemap.hpp>

namespace lucaria {
	
struct component_animator;
struct component_transform;

namespace detail {

    struct system_dynamics;

    struct system_rendering {
        system_rendering() = default;
        system_rendering(const system_rendering& other) = delete;
        system_rendering& operator=(const system_rendering& other) = delete;
        system_rendering(system_rendering&& other) = delete;
        system_rendering& operator=(system_rendering&& other) = delete;

        const std::vector<float32x3> skybox_positions = {
            float32x3(-1.f, -1.f, -1.f),
            float32x3(1.f, -1.f, -1.f),
            float32x3(1.f, 1.f, -1.f),
            float32x3(-1.f, 1.f, -1.f),
            float32x3(-1.f, -1.f, 1.f),
            float32x3(1.f, -1.f, 1.f),
            float32x3(1.f, 1.f, 1.f),
            float32x3(-1.f, 1.f, 1.f)
        };

        const std::vector<uint32x3> skybox_indices = {
            uint32x3(0, 1, 2),
            uint32x3(0, 2, 3),
            uint32x3(4, 6, 5),
            uint32x3(4, 7, 6),
            uint32x3(0, 3, 7),
            uint32x3(0, 7, 4),
            uint32x3(1, 5, 6),
            uint32x3(1, 6, 2),
            uint32x3(3, 2, 6),
            uint32x3(3, 6, 7),
            uint32x3(0, 5, 1),
            uint32x3(0, 4, 5)
        };

        float32 mouse_sensitivity = 0.05f;
        float32 player_speed = 1.f;
        float32x3 player_position = { 0.f, 1.8f, 3.f };
        float32x3 player_forward = { 0.f, 0.f, -1.f };
        float32x3 player_up = { 0.f, 1.f, 0.f };
        float32 player_pitch = 0.f;
        float32 player_yaw = 0.f;
        component_transform* _follow = nullptr;
        component_animator* _follow_animator = nullptr;
        std::string _follow_bone_name = {};
        float32x4 clear_color = { 1.f, 1.f, 1.f, 1.f };
        bool clear_depth = true;
        float32 camera_fov = 60.f;
        float32 camera_near = 0.1f;
        float32 camera_far = 1000.f;
        float32 _camera_yaw = 0.f;
        float32 _camera_pitch = 0.f;
        float32x4x4 camera_projection;
        float32x4x4 camera_view;
        float32x4x4 camera_view_projection;
        handle_cubemap skybox_cubemap = {};
        float32 _skybox_rotation = 0.f;
        bool show_free_camera = false;
        std::optional<detail::object_program> _persistent_unlit_program = std::nullopt;

        // post processing
        std::optional<detail::object_framebuffer> scene_framebuffer;
        std::optional<detail::object_texture> scene_color_texture = std::nullopt;
        std::optional<detail::object_renderbuffer> scene_depth_renderbuffer;

        // fxaa
        bool fxaa_enable = false;
        float32 fxaa_contrast_threshold = 0.0312f;
        float32 fxaa_relative_threshold = 0.125f;
        float32 fxaa_edge_sharpness = 1.5f;

#if defined(LUCARIA_DEBUG)
        object_guizmo_draw guizmo_draw = {};
        std::unordered_map<float32x3, object_mesh_line, float32x3_hash> object_mesh_linees = {};
        bool show_physics_guizmos = false;
        bool last_show_physics_guizmos_key = false;
#endif

        void update_clear_screen(manager_window& window, manager_scenes& scenes);
        void update_compute_projection(manager_window& window, manager_scenes& scenes);
        void update_apply_camera_rotation(manager_scenes& scenes);
        void update_compute_view_projection(manager_scenes& scenes);
        void update_draw_skybox(manager_scenes& scenes);
        void update_draw_blockout_meshes(manager_scenes& scenes);
        void update_draw_unlit_meshes(manager_scenes& scenes);
        void update_draw_unlit_skinned_meshes(manager_scenes& scenes);
        void update_draw_imgui_spatial_interfaces(manager_window& window, manager_input& input, manager_scenes& scenes);
        void update_draw_imgui_screen_interfaces(manager_window& window, manager_scenes& scenes);
        void update_draw_post_processing(manager_window& window, manager_scenes& scenes);
        void update_draw_debug_guizmos(system_dynamics& dynamics, manager_input& input, manager_scenes& scenes);
    };

}
}
