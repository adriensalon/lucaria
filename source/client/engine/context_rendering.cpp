#include <lucaria/engine/context_rendering.hpp>

namespace lucaria {

void context_rendering::use_skybox_cubemap(const handle_cubemap cubemap)
{
    _system->skybox_cubemap = cubemap;
}

void context_rendering::set_skybox_rotation(const float32 rotation)
{
    _system->_skybox_rotation = rotation;
}

void context_rendering::use_camera_transform(component_transform& camera)
{
    _system->_follow = &camera;
}

void context_rendering::use_camera_bone(component_animator& animator, const std::string& bone)
{
    _system->_follow_animator = &animator;
    _system->_follow_bone_name = bone;
}

void context_rendering::set_camera_fov(const float32 fov)
{
    _system->camera_fov = fov;
}

void context_rendering::set_camera_near(const float32 near)
{
    _system->camera_near = near;
}

void context_rendering::set_camera_far(const float32 far)
{
    _system->camera_far = far;
}

void context_rendering::set_camera_rotation(const float32 yaw, const float32 pitch)
{
    _system->_camera_yaw += yaw;
    _system->_camera_pitch += pitch;
}

void context_rendering::set_clear_color(const float32x4& color)
{
    _system->clear_color = color;
}

void context_rendering::set_clear_depth(const bool is_clearing)
{
    _system->clear_depth = is_clearing;
}

void context_rendering::set_fxaa_enable(const bool enable)
{
    _system->fxaa_enable = enable;
}

}