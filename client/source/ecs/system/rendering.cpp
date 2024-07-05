#include <iostream>

#include <GLES3/gl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glue/fetch.hpp>
#include <glue/window.hpp>

#include <core/program.hpp>

#include <ecs/component/model.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>
#include <ecs/system/world.hpp>

namespace detail {

static glm::vec4 clear_color = { 1.f, 1.f, 1.f, 1.f };
static bool clear_depth = true;
static float camera_fov = 60.f;
static float camera_near = 0.1f;
static float camera_far = 100.f;
static glm::mat4x4 camera_projection;

static const std::string unlit_vertex = "#version 300 es \n"
                                        "in vec3 vert_position; \n"
                                        "in vec2 vert_texcoord; \n"
                                        "uniform mat4 uniform_view; \n"
                                        "out vec2 frag_texcoord; \n"
                                        "void main() { \n"
                                        "    frag_texcoord = vert_texcoord; \n"
                                        "    gl_Position = uniform_view * vec4(vert_position, 1); \n"
                                        "}";

static const std::string unlit_fragment = "#version 300 es \n"
                                          "precision mediump float; \n"
                                          "in vec2 frag_texcoord; \n"
                                          "uniform sampler2D uniform_color; \n"
                                          "out vec4 output_color; \n"
                                          "void main() { \n"
                                          "    output_color = texture(uniform_color, frag_texcoord); \n"
                                          "}";

static const std::string skybox_vertex = "#version 300 es \n"
                                         "in vec3 vert_position; \n"
                                         "uniform mat4 uniform_projection; \n"
                                         "out vec3 frag_texcoord; \n"
                                         "void main() { \n"
                                         "    frag_texcoord = vert_position; \n"
                                         "    gl_Position = uniform_projection * vec4(vert_position, 1); \n"
                                         "};";

static const std::string skybox_fragment = "#version 300 es \n"
                                           "precision mediump float; \n"
                                           "in vec3 frag_texcoord; \n"
                                           "uniform samplerCube uniform_color; \n"
                                           "out vec4 output_color; \n"
                                           "void main() { \n"
                                           "    output_color = texture(uniform_color, frag_texcoord); \n"
                                           "};";

constexpr GLuint skybox_count = 8;

const std::vector<GLfloat> skybox_positions = {
    -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
    1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
};

const std::vector<GLuint> skybox_indices = {
    0, 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35
};

glm::mat4x4 compute_projection()
{
    glm::vec2 _screen_size = get_screen_size();
    float _fov_rad = glm::radians(detail::camera_fov);
    float _aspect_ratio = _screen_size.x / _screen_size.y;
    return glm::perspective(_fov_rad, _aspect_ratio, detail::camera_near, detail::camera_far);
}

static void clear_screen(const glm::vec4& color, const bool depth)
{
    GLbitfield _bits = depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
    glm::vec2 _screen_size = get_screen_size();
    glViewport(0, 0, _screen_size.x, _screen_size.y);
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(_bits);
}

static void draw_skybox(cubemap_ref& cubemap)
{
    // static std::optional<mesh_ref> _skybox_mesh = std::nullopt;
    // static std::optional<cubemap_ref> _skybox_cubemap = std::nullopt;
    // static std::optional<program_ref> _skybox_program = std::nullopt;
    // if (!_skybox_program.has_value()) {
    //     _skybox_program = program_ref(shader_data { skybox_vertex }, shader_data { skybox_fragment });
    // }
    // program_ref& _skybox_program_value = _skybox_program.value();
    // _skybox_program_value.use();
    // _skybox_program_value.bind("vert_position", detail::skybox_mesh.value(), mesh_attribute::position);
    // _skybox_program_value.bind("vert_texcoord", detail::skybox_mesh.value(), mesh_attribute::texcoord);
    // _skybox_program_value.bind("uniform_color", detail::skybox_cubemap.value(), 0);
    // _skybox_program_value.draw();
}

static void draw_unlit(mesh_ref& mesh, texture_ref& color, const glm::mat4x4& mvp)
{
    static std::optional<program_ref> _unlit_program = std::nullopt;
    if (!_unlit_program.has_value()) {
        _unlit_program = program_ref(shader_data { unlit_vertex }, shader_data { unlit_fragment });
    }
    program_ref& _unlit_program_value = _unlit_program.value();
    _unlit_program_value.use();
    _unlit_program_value.bind("vert_position", mesh, mesh_attribute::position);
    _unlit_program_value.bind("vert_texcoord", mesh, mesh_attribute::texcoord);
    _unlit_program_value.bind("uniform_color", color, 0);
    _unlit_program_value.bind("uniform_view", mvp);
    _unlit_program_value.draw();
}

}

void rendering_system::camera_projection(const float fov, const float near, const float far)
{
    detail::camera_fov = fov;
    detail::camera_near = near;
    detail::camera_far = far;
}

void rendering_system::clear_color(const glm::vec4& color)
{
    detail::clear_color = color;
}

void rendering_system::clear_depth(const bool clear)
{
    detail::clear_depth = clear;
}

void rendering_system::cubemap_skybox(std::future<cubemap_data>&& cubemap)
{
}

void rendering_system::update()
{
    detail::camera_projection = detail::compute_projection();
    detail::clear_screen(detail::clear_color, detail::clear_depth);
    glm::mat4x4 _view_projection = detail::camera_projection * player_system::get_view();
    world_system::for_each([&_view_projection](entt::registry& _registry) {
        _registry.view<model_component>().each([&_view_projection](model_component& _model) {
            if (_model._mesh.has_value() && _model._textures[model_texture::color].has_value()) {
                detail::draw_unlit(_model._mesh.value(), _model._textures[model_texture::color].value(), _view_projection);
            }
        });
    });
}

glm::mat4x4 rendering_system::get_projection()
{
    return detail::camera_projection;
}
