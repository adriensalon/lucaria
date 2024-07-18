#include <iostream>

#include <GLES3/gl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <core/fetch.hpp>
#include <core/program.hpp>
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/component/collider.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>

namespace detail {

static glm::vec4 clear_color = { 1.f, 1.f, 1.f, 1.f };
static bool clear_depth = true;
static float camera_fov = 60.f;
static float camera_near = 0.1f;
static float camera_far = 100.f;
static glm::mat4x4 camera_projection;

static const std::string unlit_vertex = R"(#version 300 es
    in vec3 vert_position;
    in vec2 vert_texcoord;
    uniform mat4 uniform_view;
    out vec2 frag_texcoord;
    void main() {
        frag_texcoord = vert_texcoord;
        gl_Position = uniform_view * vec4(vert_position, 1);
    })";

static const std::string unlit_fragment = R"(#version 300 es
    precision mediump float;
    in vec2 frag_texcoord;
    uniform sampler2D uniform_color;
    out vec4 output_color;
    void main() {
        output_color = texture(uniform_color, frag_texcoord);
    })";

static const std::string blockout_vertex = R"(#version 300 es
    in vec3 vert_position;
    uniform mat4 uniform_view;
    out vec3 frag_position;
    void main() {
        frag_position = vert_position;
        gl_Position = uniform_view * vec4(vert_position, 1.0);
    })";

static const std::string blockout_fragment = R"(#version 300 es
    precision mediump float;
    in vec3 frag_position;
    out vec4 output_color;
    void main() {
        vec3 abs_position = abs(frag_position);
        vec3 blend = normalize(max(abs_position, 0.00001));
        blend /= (abs_position.x + abs_position.y + abs_position.z);
        float grid_scale = 1.0;
        vec3 scaled_position = frag_position * grid_scale;
        float line_thickness = 0.7;
        vec3 grid = abs(fract(scaled_position - 0.5) - 0.5) / fwidth(scaled_position);
        float grid_factor = min(min(grid.x, grid.y), grid.z);
        vec3 base_color = vec3(0.5); // Grey color
        vec3 line_color = vec3(1.0); // White color
        float grid_line = smoothstep(0.0, line_thickness, grid_factor);
        vec3 final_color = mix(line_color, base_color, grid_line);    
        output_color = vec4(final_color, 1.0);
    })";

static const std::string pbr_vertex = R"(#version 300 es
    )";

static const std::string pbr_fragment = R"(#version 300 es
    )";

static const std::string skybox_vertex = R"(#version 300 es
    in vec3 vert_position;
    uniform mat4 uniform_projection;
    out vec3 frag_texcoord;
    void main() {
        frag_texcoord = vert_position;
        gl_Position = uniform_projection * vec4(vert_position, 1);
    })";

static const std::string skybox_fragment = R"(#version 300 es
    precision mediump float;
    in vec3 frag_texcoord;
    uniform samplerCube uniform_color;
    out vec4 output_color;
    void main() {
        output_color = texture(uniform_color, frag_texcoord);
    })";

#if LUCARIA_GUIZMO
static const std::string guizmo_collider_vertex = R"(#version 300 es
    in vec3 vert_position;
    uniform mat4 uniform_mvp;
    void main() {
        gl_Position = uniform_mvp * vec4(vert_position, 1.0);
    })";

static const std::string guizmo_collider_fragment = R"(#version 300 es
    precision mediump float;
    out vec4 output_color;
    void main() {
        output_color = vec4(0.0, 1.0, 1.0, 1.0); // Cyan color
    })";
#endif

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

}

void rendering_system::use_camera_projection(const float fov, const float near, const float far)
{
    detail::camera_fov = fov;
    detail::camera_near = near;
    detail::camera_far = far;
}

void rendering_system::use_clear_color(const glm::vec4& clear_color)
{
    detail::clear_color = clear_color;
}

void rendering_system::use_clear_depth(const bool is_clearing)
{
    detail::clear_depth = is_clearing;
}

void rendering_system::use_skybox_cubemap(const std::shared_future<std::shared_ptr<cubemap_ref>>& fetched_cubemap)
{
}

void rendering_system::clear_screen()
{
    GLbitfield _bits = detail::clear_depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
    glm::vec2 _screen_size = get_screen_size();
    glViewport(0, 0, _screen_size.x, _screen_size.y);
    glClearColor(detail::clear_color.x, detail::clear_color.y, detail::clear_color.z, detail::clear_color.w);
    glClear(_bits);
}

void rendering_system::compute_projection()
{
    glm::vec2 _screen_size = get_screen_size();
    float _fov_rad = glm::radians(detail::camera_fov);
    float _aspect_ratio = _screen_size.x / _screen_size.y;
    detail::camera_projection = glm::perspective(_fov_rad, _aspect_ratio, detail::camera_near, detail::camera_far);
}

void rendering_system::draw_skybox()
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

void rendering_system::draw_meshes()
{
    static bool _is_programs_setup = false;
    static std::optional<program_ref> _persistent_unlit_program = std::nullopt;
    static std::optional<program_ref> _persistent_blockout_program = std::nullopt;
#if LUCARIA_GUIZMO
    static std::optional<program_ref> _persistent_guizmo_collider_program = std::nullopt;
#endif
    if (!_is_programs_setup) {
        _persistent_unlit_program = program_ref(shader_data { detail::unlit_vertex }, shader_data { detail::unlit_fragment });
        _persistent_blockout_program = program_ref(shader_data { detail::blockout_vertex }, shader_data { detail::blockout_fragment });
#if LUCARIA_GUIZMO
        _persistent_guizmo_collider_program = program_ref(shader_data { detail::guizmo_collider_vertex }, shader_data { detail::guizmo_collider_fragment });
#endif
        _is_programs_setup = true;
    }
    program_ref& _unlit_program = _persistent_unlit_program.value();
    program_ref& _blockout_program = _persistent_blockout_program.value();
#if LUCARIA_GUIZMO
    program_ref& _guizmo_collider_program = _persistent_guizmo_collider_program.value();
#endif

    glm::mat4x4 _view_projection = detail::camera_projection * player_system::get_view();

    each_level([&](entt::registry& _registry) {
        _registry.view<model_component<model_shader::unlit>, transform_component>().each([&_unlit_program, &_view_projection](model_component<model_shader::unlit>& _model, transform_component& _transform) {
            if (_model._mesh.has_value() && _model._material.has_value() && _model._material.value().get_has_texture(material_texture::color)) {
                const glm::mat4 _model_view_projection = _view_projection * _transform._transform;
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._material.value().get_texture(material_texture::color);
                _unlit_program.use();
                _unlit_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind("uniform_color", _color, 0);
                _unlit_program.bind("uniform_view", _model_view_projection);
                _unlit_program.draw();
            }
        });
        _registry.view<model_component<model_shader::unlit>>(entt::exclude<transform_component>).each([&_unlit_program, &_view_projection](model_component<model_shader::unlit>& _model) {
            if (_model._mesh.has_value() && _model._material.has_value() && _model._material.value().get_has_texture(material_texture::color)) {
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._material.value().get_texture(material_texture::color);
                _unlit_program.use();
                _unlit_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind("uniform_color", _color, 0);
                _unlit_program.bind("uniform_view", _view_projection);
                _unlit_program.draw();
            }
        });

        _registry.view<model_component<model_shader::blockout>, transform_component>().each([&_blockout_program, &_view_projection](model_component<model_shader::blockout>& _model, transform_component& _transform) {
            if (_model._mesh.has_value()) {
                const glm::mat4 _model_view_projection = _view_projection * _transform._transform;
                const mesh_ref& _mesh = _model._mesh.value();
                _blockout_program.use();
                _blockout_program.bind("vert_position", _mesh, mesh_attribute::position);
                _blockout_program.bind("uniform_view", _model_view_projection);
                _blockout_program.draw();
            }
        });
        _registry.view<model_component<model_shader::blockout>>(entt::exclude<transform_component>).each([&_blockout_program, &_view_projection](model_component<model_shader::blockout>& _model) {
            if (_model._mesh.has_value()) {
                const mesh_ref& _mesh = _model._mesh.value();
                _blockout_program.use();
                _blockout_program.bind("vert_position", _mesh, mesh_attribute::position);
                _blockout_program.bind("uniform_view", _view_projection);
                _blockout_program.draw();
            }
        });

#if LUCARIA_GUIZMO
        _registry.view<collider_component<collider_algorithm::ground>>().each([&](collider_component<collider_algorithm::ground>& collider) {
            if (collider._navmesh.has_value()) {
                const guizmo_mesh_ref& _mesh = *(collider._navmesh.value()._guizmo.get());
                _guizmo_collider_program.use();
                _guizmo_collider_program.bind_guizmo("vert_position", _mesh);
                _guizmo_collider_program.bind("uniform_mvp", _view_projection);
                _guizmo_collider_program.draw_guizmo();
            }
        });
        // _registry.view<collider_component<collider_algorithm::wall>>().each([&](collider_component<collider_algorithm::wall>& collider) {
        //     const mesh_ref& _mesh = *(collider._navmesh.value()._guizmo.get());
        //     _guizmo_collider_program.use();
        //     _guizmo_collider_program.bind("vert_position", _mesh, mesh_attribute::position);
        //     _guizmo_collider_program.bind("uniform_mvp", _view_projection);
        //     _guizmo_collider_program.draw();
        // });
#endif
    });
}
