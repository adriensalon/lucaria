#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glue/window.hpp>

extern glm::mat4x4 get_projection_matrix();

namespace detail {

constexpr float mouse_sensitivity = 0.01f;
constexpr float player_speed = 0.005f;
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

}

/// @brief 
void update_controller()
{
    std::unordered_map<std::string, bool>& _keys = get_keys();
    const glm::vec2 _mouse_delta = get_mouse_position_delta();
    const float _time_delta = get_time_delta();    
    const float _forward_dir = static_cast<float>(_keys[detail::player_forward_key.data()]) - static_cast<float>(_keys[detail::player_backward_key.data()]);
    const float _right_dir = static_cast<float>(_keys[detail::player_right_key.data()]) - static_cast<float>(_keys[detail::player_left_key.data()]);
    const glm::vec3 _player_right = glm::normalize(glm::cross(detail::player_forward, detail::player_up));
    detail::player_position += _forward_dir * detail::player_speed * detail::player_forward * _time_delta;
    detail::player_position += _right_dir * detail::player_speed * _player_right * _time_delta;
    detail::player_yaw += _mouse_delta.x * detail::mouse_sensitivity * _time_delta;
    detail::player_pitch -= _mouse_delta.y * detail::mouse_sensitivity * _time_delta;
    detail::player_pitch = glm::clamp(detail::player_pitch, -89.0f, 89.0f);
    const glm::vec3 _player_direction = {
        glm::cos(glm::radians(detail::player_yaw)) * glm::cos(glm::radians(detail::player_pitch)),
        glm::sin(glm::radians(detail::player_pitch)),
        glm::sin(glm::radians(detail::player_yaw)) * glm::cos(glm::radians(detail::player_pitch))
    };
    detail::player_forward = glm::normalize(_player_direction);
    detail::player_view = glm::lookAt(detail::player_position, detail::player_position + detail::player_forward, detail::player_up);
}

/// @brief 
/// @return 
glm::mat4x4 get_view_projection_matrix()
{
    glm::mat4x4 _projection = get_projection_matrix();
    return _projection * detail::player_view;
}