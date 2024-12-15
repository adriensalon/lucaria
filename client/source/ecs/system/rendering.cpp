#include <iostream>

#include <GLES3/gl3.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <core/fetch.hpp>
#include <core/hash.hpp>
#include <core/program.hpp>
#include <core/window.hpp>
#include <core/world.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/player.hpp>
#include <ecs/system/rendering.hpp>

#if LUCARIA_GUIZMO
class guizmo_debug_draw : public btIDebugDraw {
public:
    std::unordered_map<glm::vec3, std::vector<glm::vec3>, vec3_hash> positions = {};
    std::unordered_map<glm::vec3, std::vector<glm::uvec2>, vec3_hash> indices = {};

    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        const glm::vec3 _color(color.x(), color.y(), color.z());
        std::vector<glm::vec3>& _positions = positions[_color];
        std::vector<glm::uvec2>& _indices = indices[_color];
        const glm::uint _from_index = _positions.size();
        const glm::uint _to_index = _from_index + 1;
        _positions.emplace_back(from.x(), from.y(), from.z());
        _positions.emplace_back(to.x(), to.y(), to.z());
        _indices.emplace_back(glm::uvec2(_from_index, _to_index));
    }

    virtual void reportErrorWarning(const char* warning) override
    {
        std::cout << "Bullet warning: " << warning << std::endl;
    }

    virtual void drawContactPoint(const btVector3& point_on_b, const btVector3& normal_on_b, btScalar distance, int lifetime, const btVector3& color) override
    {
        drawLine(point_on_b, point_on_b + normal_on_b * distance, color);
    }

    virtual void draw3dText(const btVector3& location, const char* text) override
    {
        std::cout << "Bullet 3D text: " << text << " at (" << location.x() << ", " << location.y() << ", " << location.z() << ")" << std::endl;
    }

    virtual void setDebugMode(int mode) override
    {
        _debug_mode = mode;
    }

    virtual int getDebugMode() const override
    {
        return _debug_mode;
    }

private:
    int _debug_mode = DBG_DrawWireframe;
};
#endif

namespace detail {

static glm::vec4 clear_color = { 1.f, 1.f, 1.f, 1.f };
static bool clear_depth = true;
static float camera_fov = 60.f;
static float camera_near = 0.1f;
static float camera_far = 1000.f;
static glm::mat4x4 camera_projection;
static glm::mat4x4 camera_view;
static glm::mat4x4 camera_view_projection;
static fetch_container<cubemap_ref> skybox_cubemap = {};

static const std::string unlit_vertex = R"(#version 300 es
    in vec3 vert_position;
    in vec2 vert_texcoord;
    uniform mat4 uniform_view;
    out vec2 frag_texcoord;
    void main() {
        frag_texcoord = vert_texcoord;
        gl_Position = uniform_view * vec4(vert_position, 1);
    })";

static const std::string unlit_skinned_vertex = R"(#version 300 es
    in vec3 vert_position;
    in vec2 vert_texcoord;
    in ivec4 vert_bones;
    in vec4 vert_weights;
    uniform mat4 uniform_view;
    uniform mat4 uniform_bones_transforms[100];
    uniform mat4 uniform_bones_invposes[100];
    out vec2 frag_texcoord;
    void main() {
        frag_texcoord = vert_texcoord;
        vec4 skinned_position = vec4(0.0);
        for (int i = 0; i < 4; ++i) {
            if (vert_weights[i] > 0.0) {
                int _index = vert_bones[i];
                skinned_position += vert_weights[i] * uniform_bones_transforms[_index] * uniform_bones_invposes[_index] * vec4(vert_position, 1.0);
            }
        }
        gl_Position = uniform_view * skinned_position;
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
    in vec3 vert_normal;
    uniform mat4 uniform_view;
    out vec3 frag_position;
    out vec3 frag_normal;
    out vec3 uv_x;
    out vec3 uv_y;
    out vec3 uv_z;    
    void main() {
        frag_position = vert_position;
        frag_normal = normalize(vert_normal);
        vec3 abs_normal = abs(frag_normal);
        vec3 uv = frag_position;
        uv_x = vec3(uv.y, uv.z, abs_normal.x);
        uv_y = vec3(uv.x, uv.z, abs_normal.y);
        uv_z = vec3(uv.x, uv.y, abs_normal.z);
        gl_Position = uniform_view * vec4(vert_position, 1.0);
    })";

static const std::string blockout_fragment = R"(#version 300 es
    precision highp float;
    in vec3 frag_position;
    in vec3 frag_normal;
    in vec3 uv_x;
    in vec3 uv_y;
    in vec3 uv_z;
    out vec4 output_color;
    void main() {
        vec3 abs_normal = abs(frag_normal);
        float total = abs_normal.x + abs_normal.y + abs_normal.z;
        vec3 blend_weights = abs_normal / total;
        float grid_scale = 1.0;
        float line_thickness = 0.02;
        vec2 uv_x2 = uv_x.xy / uv_x.z;
        vec2 uv_y2 = uv_y.xy / uv_y.z;
        vec2 uv_z2 = uv_z.xy / uv_z.z;
        vec2 grid_x = abs(fract(uv_x2 / grid_scale) - 0.5);
        vec2 grid_y = abs(fract(uv_y2 / grid_scale) - 0.5);
        vec2 grid_z = abs(fract(uv_z2 / grid_scale) - 0.5);
        float grid_x_factor = min(grid_x.x, grid_x.y);
        float grid_y_factor = min(grid_y.x, grid_y.y);
        float grid_z_factor = min(grid_z.x, grid_z.y);
        float grid_factor = min(min(grid_x_factor, grid_y_factor), grid_z_factor);
        float grid_line = smoothstep(0.0, line_thickness, grid_factor);
        vec3 base_color = vec3(0.8); // Grey color for the grid background
        vec3 line_color = vec3(0.5); // White color for the grid lines
        vec3 final_color = mix(base_color, line_color, grid_line);
        output_color = vec4(final_color, 1.0);
    })";

static const std::string pbr_vertex = R"(#version 300 es
    )";

static const std::string pbr_skinned_vertex = R"(#version 300 es
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
    precision highp float;
    in vec3 frag_texcoord;
    uniform samplerCube uniform_color;
    out vec4 output_color;
    void main() {
        output_color = texture(uniform_color, frag_texcoord);
    })";

#if LUCARIA_GUIZMO
static const std::string guizmo_vertex = R"(#version 300 es
    in vec3 vert_position;
    uniform mat4 uniform_mvp;
    void main() {
        gl_Position = uniform_mvp * vec4(vert_position, 1.0);
    })";

static const std::string guizmo_fragment = R"(#version 300 es
    precision mediump float;
    uniform vec3 uniform_color;
    out vec4 output_color;
    void main() {
        output_color = vec4(uniform_color, 1.0);
    })";
#endif

const std::vector<glm::vec3> skybox_positions = {
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, -1.0f, -1.0f),
    glm::vec3(1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, 1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, -1.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, 1.0f, 1.0f)
};

const std::vector<glm::uvec3> skybox_indices = {
    glm::uvec3(0, 1, 2),
    glm::uvec3(0, 2, 3),
    glm::uvec3(4, 6, 5),
    glm::uvec3(4, 7, 6),
    glm::uvec3(0, 3, 7),
    glm::uvec3(0, 7, 4),
    glm::uvec3(1, 5, 6),
    glm::uvec3(1, 6, 2),
    glm::uvec3(3, 2, 6),
    glm::uvec3(3, 6, 7),
    glm::uvec3(0, 5, 1),
    glm::uvec3(0, 4, 5)
};

static bool show_physics_guizmos = false;
static bool last_show_physics_guizmos_key = false;

static bool show_performance_metrics = false;
static bool last_performance_metrics_key = false;

// #if LUCARIA_DEBUG
void draw_debug_gui()
{
    static bool show_debug_gui = false;
    static bool last_show_gui_key = false;
    if (!last_show_gui_key && get_keys()["p"]) {
        show_debug_gui = !show_debug_gui;
    }
    last_show_gui_key = get_keys()["p"];
    if (show_debug_gui) {

        if (!last_show_physics_guizmos_key && get_keys()["o"]) {
            show_physics_guizmos = !show_physics_guizmos;
        }
        last_show_physics_guizmos_key = get_keys()["o"];
        ImGui::Begin("debug features [P]", nullptr);
        ImGui::Checkbox("show guizmos [O]", &show_physics_guizmos);
        ImGui::Checkbox("show performance [I]", &show_performance_metrics);
        ImGui::End();
    }
}
// #endif

#if LUCARIA_GUIZMO
extern btDiscreteDynamicsWorld* dynamics_world;
static guizmo_debug_draw guizmo_draw = {};
static std::unordered_map<glm::vec3, guizmo_mesh_ref, vec3_hash> guizmo_meshes = {};

void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color)
{
    guizmo_draw.drawLine(from, to, color);
}
#endif

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
    detail::skybox_cubemap.emplace(fetched_cubemap);
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

void rendering_system::compute_view_projection()
{
    detail::camera_view = player_system::get_view();
    detail::camera_view_projection = detail::camera_projection * detail::camera_view;
}

void rendering_system::draw_skybox()
{
    if (detail::skybox_cubemap.has_value()) {
        static bool _is_skybox_setup = false;
        static std::optional<mesh_ref> _persistent_skybox_mesh = std::nullopt;
        static std::optional<program_ref> _persistent_skybox_program = std::nullopt;
        if (!_is_skybox_setup) {
            geometry_data _skybox_geometry;
            _skybox_geometry.positions = detail::skybox_positions;
            _skybox_geometry.indices = detail::skybox_indices;
            _persistent_skybox_mesh = mesh_ref(_skybox_geometry);
            _persistent_skybox_program = program_ref(shader_data { detail::skybox_vertex }, shader_data { detail::skybox_fragment });
            _is_skybox_setup = true;
        }
        mesh_ref& _skybox_mesh = _persistent_skybox_mesh.value();
        program_ref& _skybox_program = _persistent_skybox_program.value();
        cubemap_ref& _skybox_cubemap = detail::skybox_cubemap.value();
        const glm::mat4 _no_translation_view_projection = detail::camera_projection * glm::mat4(glm::mat3(detail::camera_view));
        _skybox_program.use();
        _skybox_program.bind("vert_position", _skybox_mesh, mesh_attribute::position);
        _skybox_program.bind("uniform_color", _skybox_cubemap, 0);
        _skybox_program.bind("uniform_projection", _no_translation_view_projection);
        _skybox_program.draw(false);
    }
}

void rendering_system::draw_blockout_meshes()
{
    static bool _is_program_setup = false;
    static std::optional<program_ref> _persistent_blockout_program = std::nullopt;
    if (!_is_program_setup) {
        _persistent_blockout_program = program_ref(shader_data { detail::blockout_vertex }, shader_data { detail::blockout_fragment });
        _is_program_setup = true;
    }
    program_ref& _blockout_program = _persistent_blockout_program.value();
    each_level([&](entt::registry& _registry) {
        _registry.view<model_component<model_shader::blockout>, transform_component>().each([&](model_component<model_shader::blockout>& _model, transform_component& _transform) {
            if (_model._mesh.has_value()) {
                const glm::mat4 _model_view_projection = detail::camera_view_projection * _transform._transform;
                const mesh_ref& _mesh = _model._mesh.value();
                _blockout_program.use();
                _blockout_program.bind("vert_position", _mesh, mesh_attribute::position);
                _blockout_program.bind("vert_normal", _mesh, mesh_attribute::normal);
                _blockout_program.bind("uniform_view", _model_view_projection);
                _blockout_program.draw();
            }
        });
        _registry.view<model_component<model_shader::blockout>>(entt::exclude<transform_component>).each([&](model_component<model_shader::blockout>& _model) {
            if (_model._mesh.has_value()) {
                const mesh_ref& _mesh = _model._mesh.value();
                _blockout_program.use();
                _blockout_program.bind("vert_position", _mesh, mesh_attribute::position);
                _blockout_program.bind("vert_normal", _mesh, mesh_attribute::normal);
                _blockout_program.bind("uniform_view", detail::camera_view_projection);
                _blockout_program.draw();
            }
        });
    });
}

void rendering_system::draw_unlit_meshes()
{
    static bool _is_program_setup = false;
    static std::optional<program_ref> _persistent_unlit_program = std::nullopt;
    static std::optional<program_ref> _persistent_unlit_skinned_program = std::nullopt;
    if (!_is_program_setup) {
        _persistent_unlit_program = program_ref(shader_data { detail::unlit_vertex }, shader_data { detail::unlit_fragment });
        _persistent_unlit_skinned_program = program_ref(shader_data { detail::unlit_skinned_vertex }, shader_data { detail::unlit_fragment });
        _is_program_setup = true;
    }
    program_ref& _unlit_program = _persistent_unlit_program.value();
    program_ref& _unlit_skinned_program = _persistent_unlit_skinned_program.value();
    each_level([&](entt::registry& _registry) {
        _registry.view<model_component<model_shader::unlit>, transform_component, animator_component>().each([&](model_component<model_shader::unlit>& _model, transform_component& _transform, animator_component& animator) {
            if (_model._mesh.has_value() && _model._color.has_value() && animator._skeleton.has_value()) {
                const glm::mat4 _model_view_projection = detail::camera_view_projection * _transform._transform;
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._color.value();
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_skinned_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_skinned_program.bind("vert_bones", _mesh, mesh_attribute::bones);
                _unlit_skinned_program.bind("vert_weights", _mesh, mesh_attribute::weights);
                _unlit_skinned_program.bind("uniform_view", _model_view_projection);
                _unlit_skinned_program.bind("uniform_bones_invposes[0]", _mesh.get_invposes()); // std::vector<glm::mat4>
                _unlit_skinned_program.bind("uniform_bones_transforms[0]", animator._model_transforms); // std::vector<ozz::math::Float4x4>
                _unlit_skinned_program.bind("uniform_color", _color, 0);
                _unlit_skinned_program.draw();
            }
        });
        _registry.view<model_component<model_shader::unlit>, transform_component>(entt::exclude<animator_component>).each([&](model_component<model_shader::unlit>& _model, transform_component& _transform) {
            if (_model._mesh.has_value() && _model._color.has_value()) {
                const glm::mat4 _model_view_projection = detail::camera_view_projection * _transform._transform;
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._color.value();
                _unlit_program.use();
                _unlit_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind("uniform_color", _color, 0);
                _unlit_program.bind("uniform_view", _model_view_projection);
                _unlit_program.draw();
            }
        });
        _registry.view<model_component<model_shader::unlit>, animator_component>(entt::exclude<transform_component>).each([&](model_component<model_shader::unlit>& _model, animator_component& animator) {
            if (_model._mesh.has_value() && _model._color.has_value() && animator._skeleton.has_value()) {
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._color.value();
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_skinned_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_skinned_program.bind("vert_bones", _mesh, mesh_attribute::bones);
                _unlit_skinned_program.bind("vert_weights", _mesh, mesh_attribute::weights);
                _unlit_skinned_program.bind("uniform_color", _color, 0);
                _unlit_skinned_program.bind("uniform_view", detail::camera_view_projection);
                _unlit_skinned_program.bind("uniform_bones_transforms[0]", animator._model_transforms); // std::vector<ozz::math::Float4x4>
                _unlit_skinned_program.bind("uniform_bones_invposes[0]", _mesh.get_invposes()); // std::vector<glm::mat4>
                _unlit_skinned_program.draw();
            }
        });
        _registry.view<model_component<model_shader::unlit>>(entt::exclude<transform_component, animator_component>).each([&](model_component<model_shader::unlit>& _model) {
            if (_model._mesh.has_value() && _model._color.has_value()) {
                const mesh_ref& _mesh = _model._mesh.value();
                const texture_ref& _color = _model._color.value();
                _unlit_program.use();
                _unlit_program.bind("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind("uniform_color", _color, 0);
                _unlit_program.bind("uniform_view", detail::camera_view_projection);
                _unlit_program.draw();
            }
        });
    });
}

// void rendering_system::draw_pbr_meshes()
// {
//     // TODO
// }

void rendering_system::clear_debug_guizmos()
{
#if LUCARIA_GUIZMO
    for (std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : detail::guizmo_draw.positions) {
        const glm::vec3 _color = _pair.first;
        std::vector<glm::vec3>& _positions = _pair.second;
        std::vector<glm::uvec2>& _indices = detail::guizmo_draw.indices.at(_color);
        _positions.clear();
        _indices.clear();
    }
    detail::dynamics_world->setDebugDrawer(&detail::guizmo_draw);
#endif
}

void rendering_system::draw_debug2()
{
    // #if LUCARIA_DEBUG
    detail::draw_debug_gui();

    // #endif
    if (detail::show_physics_guizmos) {
#if LUCARIA_GUIZMO
        for (const std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : detail::guizmo_draw.positions) {
            const glm::vec3& _color = _pair.first;
            const std::vector<glm::vec3>& _positions = _pair.second;
            const std::vector<glm::uvec2>& _indices = detail::guizmo_draw.indices.at(_color);
            if (detail::guizmo_meshes.find(_color) == detail::guizmo_meshes.end()) {
                detail::guizmo_meshes.emplace(_color, guizmo_mesh_ref(_positions, _indices));
            } else {
                detail::guizmo_meshes.at(_color).update(_positions, _indices);
            }
        }
        static bool _is_program_setup = false;
        static std::optional<program_ref> _persistent_guizmo_program = std::nullopt;
        if (!_is_program_setup) {
            _persistent_guizmo_program = program_ref(shader_data { detail::guizmo_vertex }, shader_data { detail::guizmo_fragment });
            _is_program_setup = true;
        }
        program_ref& _guizmo_program = _persistent_guizmo_program.value();
        _guizmo_program.use();
        for (const std::pair<const glm::vec3, guizmo_mesh_ref>& _pair : detail::guizmo_meshes) {
            _guizmo_program.bind_guizmo("vert_position", _pair.second);
            _guizmo_program.bind("uniform_color", _pair.first);
            _guizmo_program.bind("uniform_mvp", detail::camera_view_projection);
            _guizmo_program.draw_guizmo();
        }
#endif
    }
}