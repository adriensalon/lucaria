#include <glm/glm.hpp>

extern glm::vec2 get_mouse_position_delta();
extern float get_time_delta();

namespace detail {
    
}

void update_controller()
{    
    const glm::vec2 _mouse_delta = get_mouse_position_delta();
    const float _time_delta = get_time_delta();
}