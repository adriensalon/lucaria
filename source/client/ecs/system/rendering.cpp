#include <iostream>

#include <backends/imgui_impl_opengl3.h>
#include <btBulletDynamicsCommon.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#if !defined(__EMSCRIPTEN__)
#include <backends/imgui_impl_glfw.h>
#endif

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/hash.hpp>
#include <lucaria/core/opengl.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/animator.hpp>
#include <lucaria/ecs/component/collider.hpp>
#include <lucaria/ecs/component/interface.hpp>
#include <lucaria/ecs/component/model.hpp>
#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/component/transform.hpp>
#include <lucaria/ecs/system/rendering.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* dynamics_world;
}

namespace {

#if LUCARIA_GUIZMO

    struct guizmo_debug_draw : public btIDebugDraw {

        std::unordered_map<glm::vec3, std::vector<glm::vec3>, detail::vec3_hash> positions = {};
        std::unordered_map<glm::vec3, std::vector<glm::uvec2>, detail::vec3_hash> indices = {};

        virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
        {
            const glm::vec3 _color(color.x(), color.y(), color.z());
            std::vector<glm::vec3>& _positions = positions[_color];
            std::vector<glm::uvec2>& _indices = indices[_color];
            const glm::uint _from_index = static_cast<glm::uint>(_positions.size());
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

    constexpr float mouse_sensitivity = 0.15f;
    constexpr float player_speed = 1.f;
    static glm::vec3 player_position = { 0.0f, 1.8f, 3.0f };
    static glm::vec3 player_forward = { 0.0f, 0.0f, -1.0f };
    static glm::vec3 player_up = { 0.0f, 1.0f, 0.0f };
    static float player_pitch = 0.f;
    static float player_yaw = 0.f;
    static ecs::transform_component* _follow = nullptr;
    static ecs::animator_component* _follow_animator = nullptr;
    static std::string _follow_bone_name = {};
    static glm::vec4 clear_color = { 1.f, 1.f, 1.f, 1.f };
    static bool clear_depth = true;
    static float camera_fov = 60.f;
    static float camera_near = 0.1f;
    static float camera_far = 1000.f;
    static glm::mat4x4 camera_projection;
    static glm::mat4x4 camera_view;
    static glm::mat4x4 camera_view_projection;
    static detail::fetched_container<cubemap> skybox_cubemap = {};
    static bool show_free_camera = false;

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

    extern bool show_free_camera;
    static bool last_free_camera_key = false;

    // #if LUCARIA_DEBUG
    void draw_debug_gui()
    {
        static bool show_debug_gui = false;
        static bool last_show_gui_key = false;
        if (!last_show_gui_key && get_keys()[keyboard_key::p]) {
            show_debug_gui = !show_debug_gui;
        }
        last_show_gui_key = get_keys()[keyboard_key::p];
        if (show_debug_gui) {

            if (!last_show_physics_guizmos_key && get_keys()[keyboard_key::o]) {
                show_physics_guizmos = !show_physics_guizmos;
            }
            if (!last_performance_metrics_key && get_keys()[keyboard_key::i]) {
                show_performance_metrics = !show_performance_metrics;
            }
            if (!last_free_camera_key && get_keys()[keyboard_key::u]) {
                show_free_camera = !show_free_camera;
            }
            last_show_physics_guizmos_key = get_keys()[keyboard_key::o];
            last_performance_metrics_key = get_keys()[keyboard_key::i];
            last_free_camera_key = get_keys()[keyboard_key::u];
            ImGui::Begin("Debug features [P]", nullptr, ImGuiWindowFlags_NoCollapse);
            ImGui::Checkbox("Show guizmos [O]", &show_physics_guizmos);
            ImGui::Checkbox("Show performance [I]", &show_performance_metrics);
            ImGui::Checkbox("Free camera [U]", &show_free_camera);
            ImGui::End();
        }
    }
    // #endif

#if LUCARIA_GUIZMO
    static guizmo_debug_draw guizmo_draw = {};
    static std::unordered_map<glm::vec3, guizmo_mesh, detail::vec3_hash> guizmo_meshes = {};

    void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        guizmo_draw.drawLine(from, to, color);
    }
#endif

    // static glm::vec3 compute_position()
    // {
    //     glm::vec3 _position = player_position;
    //     float _forward_dir = 0.f, _right_dir = 0.f;
    //     if (get_is_mouse_locked()) {
    //         std::unordered_map<keyboard_key, bool>& _keys = get_keys();
    //         _forward_dir = static_cast<float>(_keys[keyboard_key::w]) - static_cast<float>(_keys[keyboard_key::s]);
    //         _right_dir = static_cast<float>(_keys[keyboard_key::d]) - static_cast<float>(_keys[keyboard_key::a]);
    //     }
    //     const glm::vec3 _player_right = glm::normalize(glm::cross(player_forward, player_up));
    //     const float _time_delta = get_time_delta();
    //     _position += _forward_dir * player_speed * player_forward * _time_delta;
    //     _position += _right_dir * player_speed * _player_right * _time_delta;
    //     return _position;
    // }

    // static void compute_rotation()
    // {
    //     if (get_is_mouse_locked()) {
    //         const glm::vec2 _mouse_delta = get_mouse_position_delta();
    //         const double _time_delta = get_time_delta();
    //         player_yaw += _mouse_delta.x * mouse_sensitivity * static_cast<glm::float32>(_time_delta);
    //         player_pitch -= _mouse_delta.y * mouse_sensitivity * static_cast<glm::float32>(_time_delta);
    //     }
    //     player_pitch = glm::clamp(player_pitch, -89.0f, 89.0f);
    //     const glm::vec3 _player_direction = {
    //         glm::cos(glm::radians(player_yaw)) * glm::cos(glm::radians(player_pitch)),
    //         glm::sin(glm::radians(player_pitch)),
    //         glm::sin(glm::radians(player_yaw)) * glm::cos(glm::radians(player_pitch))
    //     };
    //     player_forward = glm::normalize(_player_direction);
    //     camera_view = glm::lookAt(player_position, player_position + player_forward, player_up);
    // }

    static glm::vec3 rotateForwardVector(const glm::vec3& forward, float pitch)
    {
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

}

namespace ecs {
    namespace rendering {

        void use_skybox_cubemap(cubemap& from)
        {
            skybox_cubemap.emplace(from);
        }

        void use_skybox_cubemap(fetched<cubemap>& from)
        {
            skybox_cubemap.emplace(from);
        }

        void use_camera_transform(transform_component& camera)
        {
            _follow = &camera;
        }

        void use_camera_bone(animator_component& animator, const std::string& bone)
        {
            _follow_animator = &animator;
            _follow_bone_name = bone;
        }

        void set_camera_projection(const float fov, const float near, const float far)
        {
            camera_fov = fov;
            camera_near = near;
            camera_far = far;
        }

        void set_clear_color(const glm::vec4& color)
        {
            clear_color = color;
        }

        void set_clear_depth(const bool is_clearing)
        {
            clear_depth = is_clearing;
        }

        glm::mat4 get_projection()
        {
            return camera_projection;
        }

        glm::mat4 get_view()
        {
            return camera_view;
        }
    }
}

namespace detail {

    void rendering_system::clear_screen()
    {
        GLbitfield _bits = clear_depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
        glm::vec2 _screen_size = get_screen_size();
        glViewport(0, 0, static_cast<GLsizei>(_screen_size.x), static_cast<GLsizei>(_screen_size.y));
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(_bits);
    }

    void rendering_system::compute_projection()
    {
        glm::vec2 _screen_size = get_screen_size();
        float _fov_rad = glm::radians(camera_fov);
        float _aspect_ratio = _screen_size.x / _screen_size.y;
        camera_projection = glm::perspective(_fov_rad, _aspect_ratio, camera_near, camera_far);
    }

    inline glm::vec3 forward_from_yaw_pitch(float yaw_deg, float pitch_deg)
    {
        float y = glm::radians(yaw_deg), p = glm::radians(pitch_deg);
        float cp = cos(p), sp = sin(p), cy = cos(y), sy = sin(y);
        // +Z forward at yaw=0, pitch=0
        return glm::normalize(glm::vec3(cp * sy, sp, cp * cy));
    }

    static void camera_basis_from_forward(
        const glm::vec3& fwd, const glm::vec3& upRef,
        glm::vec3& right, glm::vec3& up)
    {
        right = glm::normalize(glm::cross(fwd, upRef));
        up = glm::normalize(glm::cross(right, fwd));
    }

    void rendering_system::compute_view_projection()
    {

        if (get_is_mouse_locked() && _follow && _follow_animator && !_follow_bone_name.empty()) {
            const glm::vec2 md = get_mouse_position_delta();

            // ---- INPUT: use delta yaw to rotate the MODEL; accumulate only pitch for camera
            const float yawDeltaDeg = -md.x * mouse_sensitivity; // flip if you prefer
            player_pitch -= md.y * mouse_sensitivity;
            player_pitch = glm::clamp(player_pitch, -89.0f, 89.0f);

            // ---- EXTRACT CURRENT WORLD ROTATION (handles external rotations)
            const glm::mat4 followW0 = _follow->_transform;

            // Orthonormalize rotation part to avoid skew
            glm::mat3 R0 = glm::mat3(followW0);
            R0[0] = glm::normalize(R0[0]);
            R0[2] = glm::normalize(glm::cross(R0[0], glm::normalize(R0[1])));
            R0[1] = glm::normalize(glm::cross(R0[2], R0[0]));
            glm::quat qFollow0 = glm::quat_cast(glm::mat4(R0));

            // ---- ROTATE THE MODEL BY yawDelta AROUND ITS OWN UP AXIS
            const glm::vec3 charUp = glm::normalize(qFollow0 * glm::vec3(0, 1, 0));
            const glm::quat qYawDelta = glm::angleAxis(glm::radians(yawDeltaDeg), charUp);

            glm::quat qFollow1 = glm::normalize(qYawDelta * qFollow0);

            // Write back to transform (preserve translation; assuming unit scale)
            glm::mat4 followW1 = glm::mat4_cast(qFollow1);
            followW1[3] = followW0[3];
            _follow->_transform = followW1;

            // ---- RECOMPUTE after applying yaw to the model
            const glm::mat4 followW = _follow->_transform;
            glm::quat qFollow = qFollow1; // already up-to-date

            // Camera directions: pitch only relative to character forward
            const glm::vec3 camF_local = forward_from_yaw_pitch(0.0f, player_pitch);
            const glm::vec3 flatF_local = forward_from_yaw_pitch(0.0f, 0.0f); // (0,0,+1)

            const glm::vec3 camForward = glm::normalize(qFollow * camF_local);
            const glm::vec3 groundF = glm::normalize(qFollow * flatF_local);

            // Basis (keep horizon level)
            const glm::vec3 worldUp = { 0, 1, 0 };
            glm::vec3 camRight, camUp;
            camera_basis_from_forward(camForward, worldUp, camRight, camUp);

            // Bone world pos
            const glm::mat4 boneLocal = _follow_animator->get_bone_transform(_follow_bone_name);
            const glm::vec3 boneWorld = glm::vec3((followW * boneLocal)[3]);

            // Boom offset: behind character along ground heading
            const float boomDist = -0.23f; // distance behind
            const float camHeight = 0.0f; // tweak if needed

            player_position = boneWorld - groundF * boomDist + worldUp * camHeight;
            camera_view = glm::lookAt(player_position, player_position + camForward, camUp);
        }

        camera_view_projection = camera_projection * camera_view;
    }

    void rendering_system::draw_skybox()
    {
        if (skybox_cubemap.has_value()) {
            static bool _is_skybox_setup = false;
            static std::optional<mesh> _persistent_skybox_mesh = std::nullopt;
            static std::optional<program> _persistent_skybox_program = std::nullopt;

            if (!_is_skybox_setup) {
                geometry _skybox_geometry;
                _skybox_geometry.data.positions = skybox_positions;
                _skybox_geometry.data.indices = skybox_indices;

                shader _skybox_vertex_shader;
                _skybox_vertex_shader.data.text = skybox_vertex;

                shader _skybox_fragment_shader;
                _skybox_fragment_shader.data.text = skybox_fragment;

                _persistent_skybox_mesh = mesh(_skybox_geometry);
                _persistent_skybox_program = program(_skybox_vertex_shader, _skybox_fragment_shader);
                _is_skybox_setup = true;
            }

            mesh& _skybox_mesh = _persistent_skybox_mesh.value();
            program& _skybox_program = _persistent_skybox_program.value();
            cubemap& _skybox_cubemap = skybox_cubemap.value();
            const glm::mat4 _no_translation_view_projection = camera_projection * glm::mat4(glm::mat3(camera_view));
            _skybox_program.use();
            _skybox_program.bind_attribute("vert_position", _skybox_mesh, mesh_attribute::position);
            _skybox_program.bind_uniform("uniform_color", _skybox_cubemap, 0);
            _skybox_program.bind_uniform("uniform_projection", _no_translation_view_projection);
            _skybox_program.draw(false);
        }
    }

    void rendering_system::draw_blockout_meshes()
    {
        static bool _is_program_setup = false;
        static std::optional<program> _persistent_blockout_program = std::nullopt;

        if (!_is_program_setup) {
            shader _blockout_vertex_shader;
            _blockout_vertex_shader.data.text = blockout_vertex;

            shader _blockout_fragment_shader;
            _blockout_fragment_shader.data.text = blockout_fragment;

            _persistent_blockout_program = program(_blockout_vertex_shader, _blockout_fragment_shader);
            _is_program_setup = true;
        }

        program& _blockout_program = _persistent_blockout_program.value();
        each_scene([&](entt::registry& scene) {
            scene.view<ecs::model_component<ecs::model_shader::blockout>, ecs::transform_component>().each([&](ecs::model_component<ecs::model_shader::blockout>& _model, ecs::transform_component& _transform) {
                if (_model._mesh.has_value()) {
                    const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                    const mesh& _mesh = _model._mesh.value();
                    _blockout_program.use();
                    _blockout_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _blockout_program.bind_attribute("vert_normal", _mesh, mesh_attribute::normal);
                    _blockout_program.bind_uniform("uniform_view", _model_view_projection);
                    _blockout_program.draw();
                }
            });

            scene.view<ecs::model_component<ecs::model_shader::blockout>>(entt::exclude<ecs::transform_component>).each([&](ecs::model_component<ecs::model_shader::blockout>& _model) {
                if (_model._mesh.has_value()) {
                    const mesh& _mesh = _model._mesh.value();
                    _blockout_program.use();
                    _blockout_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _blockout_program.bind_attribute("vert_normal", _mesh, mesh_attribute::normal);
                    _blockout_program.bind_uniform("uniform_view", camera_view_projection);
                    _blockout_program.draw();
                }
            });
        });
    }

    static std::optional<program> _persistent_unlit_program = std::nullopt;

    void rendering_system::draw_unlit_meshes()
    {
        static bool _is_program_setup = false;

        static std::optional<program> _persistent_unlit_skinned_program = std::nullopt;
        if (!_is_program_setup) {
            shader _unlit_vertex_shader;
            shader _unlit_fragment_shader;
            _unlit_vertex_shader.data.text = unlit_vertex;
            _unlit_fragment_shader.data.text = unlit_fragment;
            _persistent_unlit_program = program(_unlit_vertex_shader, _unlit_fragment_shader);

            shader _unlit_skinned_vertex_shader;
            _unlit_skinned_vertex_shader.data.text = unlit_skinned_vertex;
            _persistent_unlit_skinned_program = program(_unlit_skinned_vertex_shader, _unlit_fragment_shader);

            _is_program_setup = true;
        }

        program& _unlit_program = _persistent_unlit_program.value();
        program& _unlit_skinned_program = _persistent_unlit_skinned_program.value();
        each_scene([&](entt::registry& scene) {
            scene.view<ecs::model_component<ecs::model_shader::unlit>, ecs::transform_component, ecs::animator_component>().each([&](ecs::model_component<ecs::model_shader::unlit>& _model, ecs::transform_component& _transform, ecs::animator_component& animator) {
                if (_model._mesh.has_value() && _model._color.has_value() && animator._skeleton.has_value()) {
                    const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                    const mesh& _mesh = _model._mesh.value();
                    const texture& _color = _model._color.value();
                    _unlit_skinned_program.use();
                    _unlit_skinned_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                    _unlit_skinned_program.bind_attribute("vert_bones", _mesh, mesh_attribute::bones);
                    _unlit_skinned_program.bind_attribute("vert_weights", _mesh, mesh_attribute::weights);
                    _unlit_skinned_program.bind_uniform("uniform_view", _model_view_projection);
                    _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.get_invposes()); // std::vector<glm::mat4>
                    _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms); // std::vector<ozz::math::Float4x4>
                    _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                    _unlit_skinned_program.draw();
                }
            });

            scene.view<ecs::model_component<ecs::model_shader::unlit>, ecs::transform_component>(entt::exclude<ecs::animator_component>).each([&](ecs::model_component<ecs::model_shader::unlit>& _model, ecs::transform_component& _transform) {
                if (_model._mesh.has_value() && _model._color.has_value()) {
                    const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                    const mesh& _mesh = _model._mesh.value();
                    const texture& _color = _model._color.value();
                    _unlit_program.use();
                    _unlit_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _unlit_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                    _unlit_program.bind_uniform("uniform_view", _model_view_projection);
                    _unlit_program.bind_uniform("uniform_color", _color, 0);
                    _unlit_program.draw();
                }
            });

            scene.view<ecs::model_component<ecs::model_shader::unlit>, ecs::animator_component>(entt::exclude<ecs::transform_component>).each([&](ecs::model_component<ecs::model_shader::unlit>& _model, ecs::animator_component& animator) {
                if (_model._mesh.has_value() && _model._color.has_value() && animator._skeleton.has_value()) {
                    const mesh& _mesh = _model._mesh.value();
                    const texture& _color = _model._color.value();
                    _unlit_skinned_program.use();
                    _unlit_skinned_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                    _unlit_skinned_program.bind_attribute("vert_bones", _mesh, mesh_attribute::bones);
                    _unlit_skinned_program.bind_attribute("vert_weights", _mesh, mesh_attribute::weights);
                    _unlit_skinned_program.bind_uniform("uniform_view", camera_view_projection);
                    _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms); // std::vector<ozz::math::Float4x4>
                    _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.get_invposes()); // std::vector<glm::mat4>
                    _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                    _unlit_skinned_program.draw();
                }
            });

            scene.view<ecs::model_component<ecs::model_shader::unlit>>(entt::exclude<ecs::transform_component, ecs::animator_component>).each([&](ecs::model_component<ecs::model_shader::unlit>& _model) {
                if (_model._mesh.has_value() && _model._color.has_value()) {
                    const mesh& _mesh = _model._mesh.value();
                    const texture& _color = _model._color.value();
                    _unlit_program.use();
                    _unlit_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                    _unlit_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                    _unlit_program.bind_uniform("uniform_color", _color, 0);
                    _unlit_program.bind_uniform("uniform_view", camera_view_projection);
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
        for (std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : guizmo_draw.positions) {
            const glm::vec3 _color = _pair.first;
            std::vector<glm::vec3>& _positions = _pair.second;
            std::vector<glm::uvec2>& _indices = guizmo_draw.indices.at(_color);
            _positions.clear();
            _indices.clear();
        }
        detail::dynamics_world->setDebugDrawer(&guizmo_draw);
#endif
    }

    void rendering_system::draw_debug_guizmos()
    {
        if (!last_show_physics_guizmos_key && get_keys()[keyboard_key::o]) {
            show_physics_guizmos = !show_physics_guizmos;
        }
        last_show_physics_guizmos_key = get_keys()[keyboard_key::o];
        // #if LUCARIA_DEBUG
        // ::draw_debug_gui();

        // #endif
        if (show_physics_guizmos) {
#if LUCARIA_GUIZMO
            for (const std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : guizmo_draw.positions) {
                const glm::vec3& _color = _pair.first;
                const std::vector<glm::vec3>& _positions = _pair.second;
                const std::vector<glm::uvec2>& _indices = guizmo_draw.indices.at(_color);
                if (guizmo_meshes.find(_color) == guizmo_meshes.end()) {
                    guizmo_meshes.emplace(_color, guizmo_mesh(_positions, _indices));
                } else {
                    guizmo_meshes.at(_color).update(_positions, _indices);
                }
            }
            static bool _is_program_setup = false;
            static std::optional<program> _persistent_guizmo_program = std::nullopt;
            if (!_is_program_setup) {
                shader _guizmo_vertex_shader;
                _guizmo_vertex_shader.data.text = guizmo_vertex;

                shader _guizmo_fragment_shader;
                _guizmo_fragment_shader.data.text = guizmo_fragment;

                _persistent_guizmo_program = program(_guizmo_vertex_shader, _guizmo_fragment_shader);
                _is_program_setup = true;
            }

            program& _guizmo_program = _persistent_guizmo_program.value();
            _guizmo_program.use();
            for (const std::pair<const glm::vec3, guizmo_mesh>& _pair : guizmo_meshes) {
                _guizmo_program.bind_guizmo("vert_position", _pair.second);
                _guizmo_program.bind_uniform("uniform_color", _pair.first);
                _guizmo_program.bind_uniform("uniform_mvp", camera_view_projection);
                _guizmo_program.draw_guizmo();
            }
#endif
        }
    }

    void rendering_system::draw_imgui_spatial_interfaces()
    {
        each_scene([](entt::registry& scene) {
            scene.view<ecs::spatial_interface_component>().each([](ecs::spatial_interface_component& interface) {
                if (interface._viewport.has_value()
                    && interface._imgui_callback
                    && (!interface._refresh_mode
                        || (interface._refresh_mode != ecs::spatial_refresh_mode::never))) {

                    interface._imgui_framebuffer->use();

                    ImGui::SetCurrentContext(interface._imgui_context);
                    const glm::uvec2 _framebuffer_size = interface._imgui_framebuffer->get_size();
                    ImGui::GetIO().DisplaySize = ImVec2(
                        static_cast<glm::float32>(_framebuffer_size.x),
                        static_cast<glm::float32>(_framebuffer_size.y));
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui::NewFrame();

                    interface._imgui_callback();

                    ImGui::SetCurrentContext(interface._imgui_context);
                    ImGui::Render();
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                    framebuffer::use_default();

                    //
                    //
                    //

                    program& _unlit_program = _persistent_unlit_program.value();

                    _unlit_program.use();
                    _unlit_program.bind_attribute("vert_position", interface._viewport.value(), mesh_attribute::position);
                    _unlit_program.bind_attribute("vert_texcoord", interface._viewport.value(), mesh_attribute::texcoord);
                    _unlit_program.bind_uniform("uniform_color", *(interface._imgui_color_texture.get()), 0);
                    _unlit_program.bind_uniform("uniform_view", camera_view_projection);
                    _unlit_program.draw();

                    if (interface._refresh_mode != ecs::spatial_refresh_mode::always) {
                        interface._refresh_mode = ecs::spatial_refresh_mode::never;
                    }
                }
            });
        });
    }

    void rendering_system::draw_imgui_screen_interfaces()
    {
        ImGui::SetCurrentContext(imgui_screen_context);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        each_scene([](entt::registry& scene) {
            scene.view<ecs::screen_interface_component>().each([](ecs::screen_interface_component& interface) {
                if (interface._imgui_callback) {
                    interface._imgui_callback();
                }
            });
        });

        ImGui::SetCurrentContext(imgui_screen_context);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

}
}