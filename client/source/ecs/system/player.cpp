#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <core/fetch.hpp>
#include <core/window.hpp>

#include <ecs/system/player.hpp>

namespace detail {

constexpr float mouse_sensitivity = 6.f;
constexpr float player_speed = 8.f;

static glm::vec3 player_position = { 0.0f, 1.8f, 3.0f };
static glm::vec3 player_forward = { 0.0f, 0.0f, -1.0f };
static glm::vec3 player_up = { 0.0f, 1.0f, 0.0f };
static float player_pitch = 0.f;
static float player_yaw = 0.f;
static glm::mat4x4 player_view;
static transform_component* _follow = nullptr;
static animator_component* _follow_animator = nullptr;
static std::string _follow_bone_name = {};

bool show_free_camera = false;

static glm::vec3 compute_position()
{
    glm::vec3 _position = player_position;
    float _forward_dir = 0.f, _right_dir = 0.f;
    if (get_is_mouse_locked()) {
        std::unordered_map<keyboard_key, bool>& _keys = get_keys();
        _forward_dir = static_cast<float>(_keys[keyboard_key::w]) - static_cast<float>(_keys[keyboard_key::s]);
        _right_dir = static_cast<float>(_keys[keyboard_key::d]) - static_cast<float>(_keys[keyboard_key::a]);
    }
    const glm::vec3 _player_right = glm::normalize(glm::cross(player_forward, player_up));
    const float _time_delta = get_time_delta();
    _position += _forward_dir * player_speed * player_forward * _time_delta;
    _position += _right_dir * player_speed * _player_right * _time_delta;
    return _position;
}

static void compute_rotation()
{
    if (get_is_mouse_locked()) {
        const glm::vec2 _mouse_delta = get_mouse_position_delta();
        const double _time_delta = get_time_delta();
        detail::player_yaw += _mouse_delta.x * detail::mouse_sensitivity * _time_delta;
        detail::player_pitch -= _mouse_delta.y * detail::mouse_sensitivity * _time_delta;
    }
    detail::player_pitch = glm::clamp(detail::player_pitch, -89.0f, 89.0f);
    const glm::vec3 _player_direction = {
        glm::cos(glm::radians(detail::player_yaw)) * glm::cos(glm::radians(detail::player_pitch)),
        glm::sin(glm::radians(detail::player_pitch)),
        glm::sin(glm::radians(detail::player_yaw)) * glm::cos(glm::radians(detail::player_pitch))
    };
    detail::player_forward = glm::normalize(_player_direction);
    detail::player_view = glm::lookAt(detail::player_position, detail::player_position + detail::player_forward, detail::player_up);
}

}

// void player_system::player_position(const glm::vec3& position)
// {
//     detail::player_position = position;
// }

// void player_system::player_direction(const glm::vec3& direction)
// {
//     detail::player_forward = direction;
// }

void player_system::follow_transform(transform_component& value)
{
    detail::_follow = &value;
}

void player_system::follow_bone(animator_component& value, const std::string& name)
{
    detail::_follow_animator = &value;
    detail::_follow_bone_name = name;
}

glm::vec3 rotateForwardVector(const glm::vec3& forward, float pitch) {
    // Clamp pitch to the range [-89, 89] degrees
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Convert pitch from degrees to radians
    float pitchRadians = glm::radians(pitch);

    // Calculate the rotation axis (cross product of forward and world-right vector)
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    // If forward is parallel to the Y-axis, use the global X-axis as a fallback
    if (glm::length(right) < 1e-6) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    // Create a quaternion representing the pitch rotation
    glm::quat pitchRotation = glm::angleAxis(pitchRadians, right);

    // Apply the rotation to the forward vector
    glm::vec3 rotatedForward = glm::normalize(pitchRotation * forward);

    return rotatedForward;
}

void player_system::update()
{
    if (detail::show_free_camera) {
        detail::compute_rotation();
        detail::player_position = detail::compute_position();
    } else {
        if (get_is_mouse_locked() && detail::_follow && detail::_follow_animator && !detail::_follow_bone_name.empty()) {
            const glm::vec2 _mouse_delta = get_mouse_position_delta();
            const double _time_delta = get_time_delta();
            detail::player_yaw -= _mouse_delta.x * detail::mouse_sensitivity * _time_delta;
            detail::player_pitch -= _mouse_delta.y * detail::mouse_sensitivity * _time_delta;
            detail::_follow->rotation_warp({ 0.f, glm::radians(detail::player_yaw), 0.f });
        detail::player_pitch = glm::clamp(detail::player_pitch, -89.0f, 89.0f);
        detail::player_position = detail::_follow->get_position() + glm::vec3(detail::_follow_animator->get_bone_transform(detail::_follow_bone_name)[3]) + detail::_follow->get_forward() * 0.23f;
        detail::player_forward = detail::_follow->get_forward();
        detail::player_forward = rotateForwardVector(detail::player_forward, detail::player_pitch);
        detail::player_view = glm::lookAt(detail::player_position, detail::player_position + detail::player_forward, detail::player_up);
        }
    }
}

glm::mat4x4 player_system::get_view()
{
    return detail::player_view;
}