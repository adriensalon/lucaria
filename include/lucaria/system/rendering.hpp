#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/component/transform.hpp>

namespace lucaria {

/// @brief Uses a cubemap object as the skybox
/// @param from the cubemap object
void use_skybox_cubemap(cubemap& from);

/// @brief Uses a cubemap object as the skybox
/// @param from the cubemap object
void use_skybox_cubemap(fetched<cubemap>& from);

/// @brief Uses a transform component as the camera
/// @param from the transform component to use
void use_camera_transform(transform_component& camera);

/// @brief Uses an animator component and a bone name as a relative offset for the camera
/// @param from the animator component to use
/// @param bone the bone name to use
void use_camera_bone(animator_component& animator, const std::string& bone);

/// @brief Sets camera projection settings
/// @param fov the FOV value in degrees
/// @param near the minimum distance to draw
/// @param far the maximum distance to draw
void set_camera_projection(
    const glm::float32 fov = 60.f, 
    const glm::float32 near = 0.1f, 
    const glm::float32 far = 100.f);

/// @brief Sets the RGBA clear color for all clear calls
/// @param color the color to use
void set_clear_color(const glm::vec4& color);

/// @brief Sets if the implemtation should clear depth for all clear calls
/// @param is_clearing if depth clear calls are enabled
void set_clear_depth(const bool is_clearing);

/// @brief 
/// @param enable 
void set_fxaa(const bool enable);

/// @brief 
/// @param contrast_threshold 
/// @param relative_threshold 
/// @param edge_sharpness 
void set_fxaa_parameters(
    const glm::float32 contrast_threshold = 0.0312f,
    const glm::float32 relative_threshold = 0.125f,
    const glm::float32 edge_sharpness = 1.5f);

namespace detail {

    struct rendering_system {
        rendering_system() = delete;
        rendering_system(const rendering_system& other) = delete;
        rendering_system& operator=(const rendering_system& other) = delete;
        rendering_system(rendering_system&& other) = delete;
        rendering_system& operator=(rendering_system&& other) = delete;

        static void clear_screen();
        static void compute_projection();
        static void compute_view_projection();
        static void draw_skybox();
        static void draw_blockout_meshes();
        static void draw_unlit_meshes();
        static void draw_imgui_spatial_interfaces();
        static void draw_imgui_screen_interfaces();
        static void draw_post_processing();
        static void draw_debug_guizmos();
    };

}
}
