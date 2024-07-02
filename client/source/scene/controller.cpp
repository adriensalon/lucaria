#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <data/mesh.hpp>
#include <glue/fetch.hpp>
#include <glue/window.hpp>

extern glm::mat4x4 get_projection_matrix();

namespace detail {

constexpr float mouse_sensitivity = 0.007f;
constexpr float player_speed = 0.002f;
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

std::vector<std::pair<glm::vec3, glm::vec3>> colliders;

std::pair<glm::vec3, glm::vec3> compute_bounding_box(const mesh_data& mesh)
{
    glm::vec3 _min_values = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 _max_values = glm::vec3(std::numeric_limits<float>::lowest());
    for (unsigned int _index = 0; _index < mesh.count; ++_index) {
        glm::vec3 vertex(
            mesh.positions[_index * 3],
            mesh.positions[_index * 3 + 1],
            mesh.positions[_index * 3 + 2]);
        _min_values = glm::min(_min_values, vertex);
        _max_values = glm::max(_max_values, vertex);
    }
    return { _min_values, _max_values };
}

bool test_position(const glm::vec3& position, const glm::vec3& min, const glm::vec3& max)
{
    return (position.x >= min.x && position.x <= max.x && position.y >= min.y && position.y <= max.y && position.z >= min.z && position.z <= max.z);
}

bool test_position(const glm::vec3& position)
{
    for (const std::pair<glm::vec3, glm::vec3>& _bounding_box : colliders) {
        const glm::vec3& _min = _bounding_box.first;
        const glm::vec3& _max = _bounding_box.second;
        if (test_position(position, _min, _max)) {
            return true;
        }
    }
    return false;
}

glm::vec3 compute_position()
{
    glm::vec3 _position = player_position;
    std::unordered_map<std::string, bool>& _keys = get_keys();
    const float _forward_dir = static_cast<float>(_keys[player_forward_key.data()]) - static_cast<float>(_keys[player_backward_key.data()]);
    const float _right_dir = static_cast<float>(_keys[player_right_key.data()]) - static_cast<float>(_keys[player_left_key.data()]);
    const glm::vec3 _player_right = glm::normalize(glm::cross(player_forward, player_up));
    const float _time_delta = get_time_delta();
    _position += _forward_dir * player_speed * player_forward * _time_delta;
    _position += _right_dir * player_speed * _player_right * _time_delta;
    return _position;
}

void compute_rotation()
{
    const glm::vec2 _mouse_delta = get_mouse_position_delta();
    const float _time_delta = get_time_delta();
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

}

void update_controller(std::future<std::vector<mesh_data>>& colliders)
{
    detail::compute_rotation();
    const glm::vec3 _new_position = detail::compute_position();
    if (!detail::test_position(_new_position)) {
        detail::player_position = _new_position;
    }
}

glm::mat4x4 get_view_projection_matrix()
{
    glm::mat4x4 _projection = get_projection_matrix();
    return _projection * detail::player_view;
}