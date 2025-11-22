#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/ecs/animator.hpp>
#include <lucaria/ecs/transform.hpp>

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
/// @param fov the FOV value in degrees (default is 60.f)
void set_camera_fov(const glm::float32 fov);

/// @brief Sets camera projection settings
/// @param near the minimum distance from camera to draw (default is 0.1f)
void set_camera_near(const glm::float32 near);

/// @brief Sets camera projection settings
/// @param far the maximum distance from camera to draw (default is 100.f)
void set_camera_far(const glm::float32 far);

/// @brief Sets the RGBA clear color for all clear calls
/// @param color the color to use
void set_clear_color(const glm::vec4& color);

/// @brief Sets if the implemtation should clear depth for all clear calls
/// @param is_clearing if depth clear calls are enabled
void set_clear_depth(const bool is_clearing);

/// @brief 
/// @param enable 
void set_fxaa_enable(const bool enable);

/// @brief 
/// @param contrast_threshold (default is 0.0312f)
void set_fxaa_contrast_threshold(const glm::float32 contrast_threshold);

/// @brief 
/// @param relative_threshold (default is 0.125f)
void set_fxaa_relative_threshold(const glm::float32 relative_threshold);

/// @brief 
/// @param edge_sharpness (default is 1.5f)
void set_fxaa_edge_sharpness(const glm::float32 edge_sharpness);

/// @brief 
/// @param enabled 
void set_guizmos(const bool enabled);

}
