#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern glm::vec2 get_mouse_position_delta();
extern float get_time_delta();
extern glm::mat4x4 get_projection_matrix();

namespace detail {

static glm::vec3 player_position;
static glm::quat player_rotation;

}

/// @brief 
void update_controller()
{    
    const glm::vec2 _mouse_delta = get_mouse_position_delta();
    const float _time_delta = get_time_delta();
    // TODO UPDATE PLAYER POS/ ROT
}

/// @brief 
/// @return 
glm::mat4x4 get_view_projection_matrix()
{
    glm::mat4x4 _projection = get_projection_matrix();
    glm::mat4x4 _rotation = glm::mat4_cast(detail::player_rotation);
    glm::mat4x4 _translation = glm::translate(glm::mat4x4(1.0f), -detail::player_position);
    glm::mat4x4 _view = _rotation * _translation;
    return _projection * _view;
}