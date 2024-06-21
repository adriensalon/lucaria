
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLES3/gl3.h>

extern glm::vec2 get_screen_size();

namespace detail {
    
extern void graphics_assert();
    
static float perspective_fov = 60.f;
static float perspective_near = 0.1f;
static float perspective_far = 100.f;
static glm::mat4x4 camera_projection = {};

void update_projection()
{
    glm::vec2 _screen_size = get_screen_size();
    float _fov_rad = glm::radians(detail::perspective_fov);
    float _aspect_ratio = _screen_size.x / _screen_size.y;
    detail::camera_projection = glm::perspective(_fov_rad, _aspect_ratio, detail::perspective_near, detail::perspective_far);
}

void clear_screen(const glm::vec4& color, const bool depth)
{
    GLbitfield _bits = depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
    glm::vec2 _screen_size = get_screen_size();
    glViewport(0, 0, _screen_size.x, _screen_size.y);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(_bits);
}

}

/// @brief Clears the default framebuffer.
/// @param color is the color to clear the color buffer with.
/// @param depth specifies if we need to clear the depth buffer too.
void update_camera(const glm::vec4& color = { 0, 0, 0, 1 }, const bool depth = true)
{
    detail::update_projection();
    detail::clear_screen(color, depth);
    detail::graphics_assert();
}

/// @brief Returns the current projection matrix defined by hardcoded values in camera.cpp
/// @return the constructed projection matrix
glm::mat4x4 get_projection_matrix() 
{
    return detail::camera_projection;
}