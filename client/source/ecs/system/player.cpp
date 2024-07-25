#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <core/fetch.hpp>
#include <core/window.hpp>

#include <ecs/system/player.hpp>
#include <ecs/system/splash.hpp>

namespace detail {

constexpr float mouse_sensitivity = 6.f;
constexpr float player_speed = 2.f;
constexpr std::string_view player_forward_key = "z";
constexpr std::string_view player_left_key = "q";
constexpr std::string_view player_backward_key = "s";
constexpr std::string_view player_right_key = "d";

static glm::vec3 player_position = { 0.0f, 1.8f, 3.0f };
static glm::vec3 player_forward = { 0.0f, 0.0f, -1.0f };
static glm::vec3 player_up = { 0.0f, 1.0f, 0.0f };
static float player_pitch = 0.f;
static float player_yaw = 0.f;
static glm::mat4x4 player_view;

static glm::vec3 compute_position()
{
    glm::vec3 _position = player_position;
    float _forward_dir, _right_dir;
    if (get_is_mouse_locked()) {
        std::unordered_map<std::string, bool>& _keys = get_keys();
        _forward_dir = static_cast<float>(_keys[player_forward_key.data()]) - static_cast<float>(_keys[player_backward_key.data()]);
        _right_dir = static_cast<float>(_keys[player_right_key.data()]) - static_cast<float>(_keys[player_left_key.data()]);
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

void player_system::player_position(const glm::vec3& position)
{
    detail::player_position = position;
}

void player_system::player_direction(const glm::vec3& direction)
{
    detail::player_forward = direction;
}

void player_system::player_height(const float height)
{

}

void player_system::player_radius(const float radius)
{

}


void player_system::update()
{
    detail::compute_rotation();
    detail::player_position = detail::compute_position();
}

glm::mat4x4 player_system::get_view()
{
    return detail::player_view;
}