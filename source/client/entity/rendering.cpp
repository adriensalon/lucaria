#include <iostream>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <backends/imgui_impl_opengl3.h>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <backends/imgui_impl_psp.h>
#endif

#include <imgui.h>

#include <lucaria/core/input.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/renderbuffer.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/entity/animator.hpp>
#include <lucaria/entity/interface.hpp>
#include <lucaria/entity/model.hpp>
#include <lucaria/entity/rendering.hpp>
#include <lucaria/entity/rigidbody.hpp>
#include <lucaria/entity/scene.hpp>
#include <lucaria/entity/transform.hpp>

namespace lucaria {

#if defined(LUCARIA_DEBUG)
void _draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

extern btDiscreteDynamicsWorld* _dynamics_world;

namespace detail {

    namespace {

#if defined(LUCARIA_DEBUG)
        struct vec3_hash {
            std::size_t operator()(const glm::vec3& vec) const
            {
                std::size_t _h1 = std::hash<glm::float32> {}(vec.x);
                std::size_t _h2 = std::hash<glm::float32> {}(vec.y);
                std::size_t _h3 = std::hash<glm::float32> {}(vec.z);
                return _h1 ^ (_h2 << 1) ^ (_h3 << 2);
            }
        };

        struct guizmo_debug_draw : public btIDebugDraw {

            std::unordered_map<glm::vec3, std::vector<glm::vec3>, vec3_hash> positions = {};
            std::unordered_map<glm::vec3, std::vector<glm::uvec2>, vec3_hash> indices = {};

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

        static guizmo_debug_draw guizmo_draw = {};
        static std::unordered_map<glm::vec3, _detail::guizmo_mesh, vec3_hash> guizmo_meshes = {};
    }
}

void _draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color)
{
    detail::guizmo_draw.drawLine(from, to, color);
}

namespace detail {
    namespace {
#endif

        constexpr glm::float32 mouse_sensitivity = 0.05f;
        constexpr glm::float32 player_speed = 1.f;
        static glm::vec3 player_position = { 0.f, 1.8f, 3.f };
        static glm::vec3 player_forward = { 0.f, 0.f, -1.f };
        static glm::vec3 player_up = { 0.f, 1.f, 0.f };
        static glm::float32 player_pitch = 0.f;
        static glm::float32 player_yaw = 0.f;
        static transform_component* _follow = nullptr;
        static animator_component* _follow_animator = nullptr;
        static std::string _follow_bone_name = {};
        static glm::vec4 clear_color = { 1.f, 1.f, 1.f, 1.f };
        static bool clear_depth = true;
        static glm::float32 camera_fov = 60.f;
        static glm::float32 camera_near = 0.1f;
        static glm::float32 camera_far = 1000.f;
        static glm::float32 _camera_yaw = 0.f;
        static glm::float32 _camera_pitch = 0.f;
        static glm::mat4x4 camera_projection;
        static glm::mat4x4 camera_view;
        static glm::mat4x4 camera_view_projection;
        static cubemap_object skybox_cubemap = {};
        static glm::float32 _skybox_rotation = 0.f;
        static bool show_free_camera = false;
        static std::optional<detail::program_implementation> _persistent_unlit_program = std::nullopt;

        // post processing
        static std::optional<detail::framebuffer_implementation> scene_framebuffer;
        static std::optional<detail::texture_implementation> scene_color_texture = std::nullopt;
        static std::optional<detail::renderbuffer_implementation> scene_depth_renderbuffer;

        // fxaa
        static bool fxaa_enable = false;
        static glm::float32 fxaa_contrast_threshold = 0.0312f;
        static glm::float32 fxaa_relative_threshold = 0.125f;
        static glm::float32 fxaa_edge_sharpness = 1.5f;

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

#if defined(LUCARIA_DEBUG)
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

        static const std::string post_processing_vertex = R"(#version 300 es
    in vec3 vert_position;
    out vec2 frag_texcoord;
    void main() {
        gl_Position = vec4(vert_position, 1.0);
        frag_texcoord = vert_position.xy * 0.5 + 0.5;
    })";

        static const std::string post_processing_fragment = R"(#version 300 es
    precision mediump float;
    in vec2 frag_texcoord;
    uniform sampler2D uniform_color;
    uniform vec2 uniform_texel_size;

    // fxaa
    uniform float uniform_fxaa_enable;
    uniform float uniform_fxaa_contrast_threshold;
    uniform float uniform_fxaa_relative_threshold;
    uniform float uniform_fxaa_edge_sharpness;

    out vec4 output_color;

    float fxaa_luma(vec3 c) {
        return dot(c, vec3(0.299, 0.587, 0.114));
    }

    void main()
    {
        // fxaa
        vec3 _rgb_m = texture(uniform_color, frag_texcoord).rgb;
        float _luma_m = fxaa_luma(_rgb_m);
        vec3 _rgb_n = texture(uniform_color, frag_texcoord + vec2(0.0, -uniform_texel_size.y)).rgb;
        vec3 _rgb_s = texture(uniform_color, frag_texcoord + vec2(0.0, uniform_texel_size.y)).rgb;
        vec3 _rgb_w = texture(uniform_color, frag_texcoord + vec2(-uniform_texel_size.x, 0.0)).rgb;
        vec3 _rgb_e = texture(uniform_color, frag_texcoord + vec2( uniform_texel_size.x, 0.0)).rgb;
        float _luma_n = fxaa_luma(_rgb_n);
        float _luma_s = fxaa_luma(_rgb_s);
        float _luma_w = fxaa_luma(_rgb_w);
        float _luma_e = fxaa_luma(_rgb_e);
        float _luma_min = min(_luma_m, min(min(_luma_n, _luma_s), min(_luma_w, _luma_e)));
        float _luma_max = max(_luma_m, max(max(_luma_n, _luma_s), max(_luma_w, _luma_e)));
        float _contrast = _luma_max - _luma_min;
        float _threshold = max(uniform_fxaa_contrast_threshold, _luma_max * uniform_fxaa_relative_threshold);
        if (_contrast < _threshold) {
            output_color = vec4(_rgb_m, 1.0);
            return;
        }
        vec2 _direction;
        _direction.x = (_luma_w - _luma_e);
        _direction.y = (_luma_n - _luma_s);
        float _direction_reduce = (abs(_direction.x) + abs(_direction.y)) + 1e-4;
        _direction /= _direction_reduce;
        _direction *= uniform_texel_size * uniform_fxaa_edge_sharpness;
        vec3 _rgb_a = texture(uniform_color, frag_texcoord + _direction * 0.5).rgb;
        vec3 _rgb_b = texture(uniform_color, frag_texcoord - _direction * 0.5).rgb;
        vec3 _aa_color = (_rgb_a + _rgb_b + _rgb_m) / 3.0;
        vec3 _final_color = mix(_rgb_m, _aa_color, uniform_fxaa_enable);

        output_color = vec4(_final_color, 1.0);
    })";

        static const std::vector<glm::vec3> skybox_positions = {
            glm::vec3(-1.f, -1.f, -1.f),
            glm::vec3(1.f, -1.f, -1.f),
            glm::vec3(1.f, 1.f, -1.f),
            glm::vec3(-1.f, 1.f, -1.f),
            glm::vec3(-1.f, -1.f, 1.f),
            glm::vec3(1.f, -1.f, 1.f),
            glm::vec3(1.f, 1.f, 1.f),
            glm::vec3(-1.f, 1.f, 1.f)
        };

        static const std::vector<glm::uvec3> skybox_indices = {
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

        static glm::vec3 forward_from_yaw_pitch(float yaw_deg, float pitch_deg)
        {
            float y = glm::radians(yaw_deg), p = glm::radians(pitch_deg);
            float cp = cos(p), sp = sin(p), cy = cos(y), sy = sin(y);
            // −Z forward at yaw=0, pitch=0
            return glm::normalize(glm::vec3(cp * sy, sp, -cp * cy));
        }

        static void camera_basis_from_forward(
            const glm::vec3& fwd, const glm::vec3& upRef,
            glm::vec3& right, glm::vec3& up)
        {
            right = glm::normalize(glm::cross(fwd, upRef));
            up = glm::normalize(glm::cross(right, fwd));
        }

        struct raycast_data {
            glm::vec3 origin;
            glm::vec3 direction;
        };

        [[nodiscard]] static bool viewport_raycast_triangle(
            const raycast_data& raycast,
            const glm::vec3& vertex_position_a,
            const glm::vec3& vertex_position_b,
            const glm::vec3& vertex_position_c,
            glm::vec3& collision_position)
        {
            const glm::float32 _eps = 1e-7f;
            const glm::vec3 _ab = vertex_position_b - vertex_position_a;
            const glm::vec3 _ac = vertex_position_c - vertex_position_a;

            const glm::vec3 _p = glm::cross(raycast.direction, _ac);
            const glm::float32 _det = glm::dot(_ab, _p);
            if (_det < _eps) {
                return false; // we cull backfaces
            }

            const glm::float32 _inv_det = 1.f / _det;
            const glm::vec3 _s = raycast.origin - vertex_position_a;
            collision_position.y = glm::dot(_s, _p) * _inv_det;
            if (collision_position.y < 0.f || collision_position.y > 1.f) {
                return false;
            }

            const glm::vec3 _q = glm::cross(_s, _ab);
            collision_position.z = glm::dot(raycast.direction, _q) * _inv_det;
            if (collision_position.z < 0.f || collision_position.x + collision_position.z > 1.f) {
                return false;
            }

            collision_position.x = glm::dot(_ac, _q) * _inv_det;
            return collision_position.x >= 0.f;
        }

        [[nodiscard]] static glm::vec2 viewport_lerp_uv(
            const glm::vec2& vertex_texcoord_a,
            const glm::vec2& vertex_texcoord_b,
            const glm::vec2& vertex_texcoord_c,
            const glm::float32 lerp_texcoord_u,
            const glm::float32 lerp_texcoord_v)
        {
            const glm::float32 _w = 1.0f - lerp_texcoord_u - lerp_texcoord_v;
            return vertex_texcoord_a * _w + vertex_texcoord_b * lerp_texcoord_u + vertex_texcoord_c * lerp_texcoord_v;
        }

        [[nodiscard]] std::optional<glm::vec2> viewport_raycast(const detail::geometry_implementation& viewport_geometry)
        {
            glm::mat4 _inverse_view = glm::inverse(camera_view);
            glm::vec3 _origin = glm::vec3(_inverse_view * glm::vec4(0, 0, 0, 1));
            glm::vec3 _direction = glm::normalize(glm::vec3(_inverse_view * glm::vec4(0, 0, -1, 0)));
            const raycast_data _raycast { _origin, _direction };
            bool _has_hit = false;
            glm::float32 _best_distance = std::numeric_limits<glm::float32>::infinity();
            glm::vec2 _best_uv = glm::vec2(0);

            // compute for each triangle
            for (const glm::uvec3& _triangle : viewport_geometry.data.indices) {
                const glm::vec3& _vertex_a = viewport_geometry.data.positions[_triangle.x];
                const glm::vec3& _vertex_b = viewport_geometry.data.positions[_triangle.y];
                const glm::vec3& _vertex_c = viewport_geometry.data.positions[_triangle.z];
                glm::vec3 _collision_position;
                if (!viewport_raycast_triangle(_raycast, _vertex_a, _vertex_b, _vertex_c, _collision_position)) {
                    continue;
                }
                if (_collision_position.x < _best_distance) {
                    const glm::vec2& _texcoord_a = viewport_geometry.data.texcoords[_triangle.x];
                    const glm::vec2& _texcoord_b = viewport_geometry.data.texcoords[_triangle.y];
                    const glm::vec2& _texcoord_c = viewport_geometry.data.texcoords[_triangle.z];
                    _best_distance = _collision_position.x;
                    _best_uv = viewport_lerp_uv(_texcoord_a, _texcoord_b, _texcoord_c, _collision_position.y, _collision_position.z);
                    _has_hit = true;
                }
            }

            if (!_has_hit) {
                return std::nullopt;
            }
            return _best_uv;
        }

        [[nodiscard]] static glm::quat extract_world_rotation(const glm::mat4& transform)
        {
            // Orthonormalize rotation part to avoid skew before converting to a quaternion.
            glm::mat3 rotation = glm::mat3(transform);
            rotation[0] = glm::normalize(rotation[0]);
            rotation[2] = glm::normalize(glm::cross(rotation[0], glm::normalize(rotation[1])));
            rotation[1] = glm::normalize(glm::cross(rotation[2], rotation[0]));
            return glm::quat_cast(glm::mat4(rotation));
        }
    }

    void rendering_system::clear_screen()
    {
        const glm::uvec2 _screen_size = engine_window().screen_size;

        // setup screen buffers
        if (!scene_framebuffer) {
            scene_framebuffer.emplace();
        }
        if (!scene_color_texture) {
            scene_color_texture.emplace(_screen_size);
            scene_framebuffer->bind_color(scene_color_texture.value());
        } else {
            scene_color_texture->resize(_screen_size);
        }
        if (!scene_depth_renderbuffer) {
#if defined(LUCARIA_BACKEND_OPENGL)
            scene_depth_renderbuffer = detail::renderbuffer_implementation(_screen_size, GL_DEPTH_COMPONENT24);
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
            // scene_depth_renderbuffer = detail::renderbuffer_implementation(_screen_size, GL_DEPTH_COMPONENT24); TODO
#endif
            scene_framebuffer->bind_depth(scene_depth_renderbuffer.value());
        } else {
            scene_depth_renderbuffer->resize(_screen_size);
        }

        // clear screen
        scene_framebuffer->use();
        detail::program_implementation::viewport(_screen_size);
        detail::program_implementation::clear(true);
    }

    void rendering_system::compute_projection()
    {
        glm::vec2 _screen_size = engine_window().screen_size;
        float _fov_rad = glm::radians(camera_fov);
        float _aspect_ratio = _screen_size.x / _screen_size.y;
        camera_projection = glm::perspective(_fov_rad, _aspect_ratio, camera_near, camera_far);
    }

    void rendering_system::apply_camera_rotation()
    {
        // if (!get_is_game_locked() || !_follow) {
        //     return;
        // }
        if (!_follow) {
            return;
        }

        const glm::vec2 _mouse_delta = glm::vec2(_camera_yaw, _camera_pitch);
        if (_mouse_delta == glm::vec2(0.f)) {
            return;
        }

        // Consume this frame's camera input before motion/physics run.
        const float _yaw_delta_degrees = -_mouse_delta.x * mouse_sensitivity;
        player_pitch -= _mouse_delta.y * mouse_sensitivity;
        player_pitch = glm::clamp(player_pitch, -89.f, 89.f);
        _camera_yaw = 0.f;
        _camera_pitch = 0.f;

        const glm::mat4 followW0 = _follow->_transform;
        const glm::quat qFollow0 = extract_world_rotation(followW0);

        // Rotate the model by yaw delta around its own up axis.
        const glm::vec3 charUp = glm::normalize(qFollow0 * glm::vec3(0, 1, 0));
        const glm::quat qYawDelta = glm::angleAxis(glm::radians(_yaw_delta_degrees), charUp);
        const glm::quat qFollow1 = glm::normalize(qYawDelta * qFollow0);

        // Preserve translation. This assumes unit scale, same as the previous code.
        glm::mat4 followW1 = glm::mat4_cast(qFollow1);
        followW1[3] = followW0[3];
        _follow->set_transform_warp(followW1);
    }

    void rendering_system::compute_view_projection()
    {
        // if (get_is_game_locked() && _follow && _follow_animator && !_follow_bone_name.empty()) {
        if (_follow && _follow_animator && !_follow_bone_name.empty()) {
            const glm::mat4 followW = _follow->_transform;
            const glm::quat qFollow = extract_world_rotation(followW);

            // Camera directions: pitch only relative to character forward.
            const glm::vec3 camF_local = forward_from_yaw_pitch(0.f, player_pitch);
            const glm::vec3 flatF_local = forward_from_yaw_pitch(0.f, 0.f);

            const glm::vec3 camForward = glm::normalize(qFollow * camF_local);
            const glm::vec3 groundF = glm::normalize(qFollow * flatF_local);

            // Basis (keep horizon level).
            const glm::vec3 worldUp = { 0, 1, 0 };
            glm::vec3 camRight, camUp;
            camera_basis_from_forward(camForward, worldUp, camRight, camUp);

            // Bone world pos. This is intentionally evaluated after mixer/animation.
            const glm::mat4 boneLocal = _follow_animator->get_bone_transform(_follow_bone_name);
            const glm::vec3 boneWorld = glm::vec3((followW * boneLocal)[3]);
            // const glm::vec3 boneWorld = glm::vec3(followW[3]);

            const float boomDist = -0.23f;
            const float camHeight = 0.f;

            player_position = boneWorld - groundF * boomDist + worldUp * camHeight;
            camera_view = glm::lookAt(player_position, player_position + camForward, camUp);
        }

        camera_view_projection = camera_projection * camera_view;
    }

    void rendering_system::draw_skybox()
    {
        if (skybox_cubemap) {
            static bool _is_skybox_setup = false;
            static std::optional<detail::mesh_implementation> _persistent_skybox_mesh = std::nullopt;
            static std::optional<detail::program_implementation> _persistent_skybox_program = std::nullopt;

            if (!_is_skybox_setup) {
                geometry_data _geometry_data;
                _geometry_data.positions = skybox_positions;
                _geometry_data.indices = skybox_indices;
                detail::geometry_implementation _skybox_geometry(std::move(_geometry_data));

                shader _skybox_vertex_shader(shader_data { skybox_vertex });
                shader _skybox_fragment_shader(shader_data { skybox_fragment });

                _persistent_skybox_mesh.emplace(_skybox_geometry);
                _persistent_skybox_program.emplace(_skybox_vertex_shader, _skybox_fragment_shader);
                _is_skybox_setup = true;
            }

            detail::mesh_implementation& _skybox_mesh = _persistent_skybox_mesh.value();
            detail::program_implementation& _skybox_program = _persistent_skybox_program.value();
            detail::cubemap_implementation& _skybox_cubemap = skybox_cubemap._resource->fetched.value();
            const glm::mat4 _skybox_rotation_matrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians(_skybox_rotation), glm::vec3(0, 1, 0));
            const glm::mat4 _no_translation_view_projection = camera_projection * glm::mat4(glm::mat3(camera_view)) * _skybox_rotation_matrix;
            _skybox_program.use();
            _skybox_program.bind_attribute("vert_position", _skybox_mesh, detail::mesh_attribute::position);
            _skybox_program.bind_uniform("uniform_color", _skybox_cubemap, 0);
            _skybox_program.bind_uniform("uniform_projection", _no_translation_view_projection);
            _skybox_program.draw(false);
        }
    }

    void rendering_system::draw_blockout_meshes()
    {
        static bool _is_program_setup = false;
        static std::optional<detail::program_implementation> _persistent_blockout_program = std::nullopt;
        if (!_is_program_setup) {
            shader _blockout_vertex_shader(shader_data { blockout_vertex });
            shader _blockout_fragment_shader(shader_data { blockout_fragment });
            _persistent_blockout_program = detail::program_implementation(_blockout_vertex_shader, _blockout_fragment_shader);
            _is_program_setup = true;
        }

        detail::program_implementation& _blockout_program = _persistent_blockout_program.value();
        each_view<blockout_model_component, transform_component>([&](blockout_model_component& _model, transform_component& _transform) {
            if (_model._mesh) {
                const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                const detail::mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                _blockout_program.use();
                _blockout_program.bind_attribute("vert_position", _mesh, detail::mesh_attribute::position);
                _blockout_program.bind_attribute("vert_normal", _mesh, detail::mesh_attribute::normal);
                _blockout_program.bind_uniform("uniform_view", _model_view_projection);
                _blockout_program.draw();
            }
        });

        each_view<blockout_model_component>(entt::exclude<transform_component>, [&](blockout_model_component& _model) {
            if (_model._mesh) {
                const detail::mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                _blockout_program.use();
                _blockout_program.bind_attribute("vert_position", _mesh, detail::mesh_attribute::position);
                _blockout_program.bind_attribute("vert_normal", _mesh, detail::mesh_attribute::normal);
                _blockout_program.bind_uniform("uniform_view", camera_view_projection);
                _blockout_program.draw();
            }
        });
    }

    void rendering_system::draw_unlit_meshes()
    {
        static bool _is_program_setup = false;
        if (!_is_program_setup) {
            shader _unlit_vertex_shader(shader_data { unlit_vertex });
            shader _unlit_fragment_shader(shader_data { unlit_fragment });
            _persistent_unlit_program = program_implementation(_unlit_vertex_shader, _unlit_fragment_shader);
            _is_program_setup = true;
        }

        program_implementation& _unlit_program = _persistent_unlit_program.value();
        each_view<unlit_model_component, transform_component>(entt::exclude<animator_component>, [&](unlit_model_component& _model, transform_component& _transform) {
            if (_model._mesh && _model._color) {
                const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                const mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                const texture_implementation& _color = _model._color._resource->fetched.value();
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_view", _model_view_projection);
                _unlit_program.bind_uniform("uniform_color", _color, 0);
                _unlit_program.draw();
            }
        });

        each_view<unlit_model_component>(entt::exclude<transform_component, animator_component>, [&](unlit_model_component& _model) {
            if (_model._mesh && _model._color) {
                const mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                const texture_implementation& _color = _model._color._resource->fetched.value();
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_color", _color, 0);
                _unlit_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_program.draw();
            }
        });
    }

    void rendering_system::draw_unlit_skinned_meshes()
    {
        static bool _is_program_setup = false;
        static std::optional<program_implementation> _persistent_unlit_skinned_program = std::nullopt;
        if (!_is_program_setup) {
            shader _unlit_fragment_shader(shader_data { unlit_fragment });
            shader _unlit_skinned_vertex_shader(shader_data { unlit_skinned_vertex });
            _persistent_unlit_skinned_program = program_implementation(_unlit_skinned_vertex_shader, _unlit_fragment_shader);
            _is_program_setup = true;
        }

        program_implementation& _unlit_skinned_program = _persistent_unlit_skinned_program.value();
        each_view<unlit_model_component, transform_component, animator_component>([&](unlit_model_component& _model, transform_component& _transform, animator_component& animator) {
            if (_model._mesh && _model._color && animator._skeleton.has_value()) {
                const glm::mat4 _model_view_projection = camera_view_projection * _transform._transform;
                const mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                const texture_implementation& _color = _model._color._resource->fetched.value();
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_skinned_program.bind_attribute("vert_bones", _mesh, mesh_attribute::bones);
                _unlit_skinned_program.bind_attribute("vert_weights", _mesh, mesh_attribute::weights);
                _unlit_skinned_program.bind_uniform("uniform_view", _model_view_projection);
                _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.invposes);
                _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms);
                _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                _unlit_skinned_program.draw();
            }
        });

        each_view<unlit_model_component, animator_component>(entt::exclude<transform_component>, [&](unlit_model_component& _model, animator_component& animator) {
            if (_model._mesh && _model._color && animator._skeleton.has_value()) {
                const mesh_implementation& _mesh = _model._mesh._resource->fetched.value();
                const texture_implementation& _color = _model._color._resource->fetched.value();
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind_attribute("vert_position", _mesh, mesh_attribute::position);
                _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, mesh_attribute::texcoord);
                _unlit_skinned_program.bind_attribute("vert_bones", _mesh, mesh_attribute::bones);
                _unlit_skinned_program.bind_attribute("vert_weights", _mesh, mesh_attribute::weights);
                _unlit_skinned_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms);
                _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.invposes);
                _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                _unlit_skinned_program.draw();
            }
        });
    }

    void rendering_system::draw_imgui_spatial_interfaces()
    {
        each_view<spatial_interface_component>([](spatial_interface_component& interface) {
            if (interface._viewport_geometry.has_value()
                && interface._viewport_mesh
                && interface._imgui_callback
                && (!interface._refresh_mode
                    || (interface._refresh_mode != refresh_mode::never))) {

                ImGui::SetCurrentContext(interface._imgui_context);
                ImGui::GetIO().DisplaySize = ImVec2(static_cast<glm::float32>(interface._viewport_size.x), static_cast<glm::float32>(interface._viewport_size.y));

                std::optional<glm::vec2> _raycasted_uvs;
                if (interface._use_interaction) {
                    _raycasted_uvs = viewport_raycast(interface._viewport_geometry._resource->fetched.value());
                    if (_raycasted_uvs) {
                        interface._interaction_screen_position = {
                            (_raycasted_uvs.value().x) * interface._viewport_size.x,
                            (1.f - _raycasted_uvs.value().y) * interface._viewport_size.y
                        };
                        ImGui::GetIO().MousePos = ImVec2(interface._interaction_screen_position.value().x, interface._interaction_screen_position.value().y);
                        ImGui::GetIO().MouseDown[0] = get_buttons()[input_key::mouse_left].state;
                    } else {
                        interface._interaction_screen_position = std::nullopt;
                    }
                }

                interface._imgui_framebuffer->use();
                program_implementation::clear(false);
#if defined(LUCARIA_BACKEND_OPENGL)
                ImGui_ImplOpenGL3_NewFrame();
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
                ImGui_ImplPSP_NewFrame();
#endif
                ImGui::NewFrame();

                interface._imgui_callback();

                if (interface._use_interaction && interface._interaction_texture.has_value()) {
                    const ImTextureID _texture_id = interface._interaction_texture.imgui_texture();
                    if (_raycasted_uvs) {
                        const ImVec2 _cursor_min(interface._interaction_screen_position.value().x, interface._interaction_screen_position.value().y);
                        const ImVec2 _cursor_max(
                            _cursor_min.x + interface._cursor_size.x,
                            _cursor_min.y + interface._cursor_size.y);

                        ImDrawList* _drawlist = ImGui::GetForegroundDrawList(); // screen space
                        _drawlist->AddImage(_texture_id, _cursor_min, _cursor_max); // UVs default (0,0)-(1,1), color = white
                    }
                }

                ImGui::SetCurrentContext(interface._imgui_context);
                ImGui::Render();
#if defined(LUCARIA_BACKEND_OPENGL)
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
                ImGui_ImplPSP_RenderDrawData(ImGui::GetDrawData());
#endif

                scene_framebuffer->use();
                program_implementation& _unlit_program = _persistent_unlit_program.value();
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", interface._viewport_mesh.value(), mesh_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", interface._viewport_mesh.value(), mesh_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_color", interface._imgui_color_texture.value(), 0);
                _unlit_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_program.draw();

                if (interface._refresh_mode != refresh_mode::always) {
                    interface._refresh_mode = refresh_mode::never;
                }
            }
        });
    }

    void rendering_system::draw_imgui_screen_interfaces()
    {
        ImGui::SetCurrentContext(engine_window().screen_context);
#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_NewFrame();
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
        ImGui_ImplPSP_NewFrame();
#endif
        ImGui::NewFrame();

        each_view<screen_interface_component>([](screen_interface_component& interface) {
            if (interface._imgui_callback) {
                interface._imgui_callback();
            }
        });

        ImGui::SetCurrentContext(engine_window().screen_context);
        ImGui::Render();
#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
        ImGui_ImplPSP_RenderDrawData(ImGui::GetDrawData());
#endif
    }

    void rendering_system::draw_post_processing()
    {
        static bool _is_post_processing_setup = false;
        static std::optional<mesh_implementation> _persistent_post_processing_mesh = std::nullopt;
        static std::optional<program_implementation> _persistent_post_processing_program = std::nullopt;

        if (!_is_post_processing_setup) {
            geometry_data _geometry_data;

            _geometry_data.positions = {
                glm::vec3(-1.f, -1.f, 0.f),
                glm::vec3(1.f, -1.f, 0.f),
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            };

            _geometry_data.indices = {
                glm::uvec3(0, 1, 2),
                glm::uvec3(0, 2, 3),
            };

            geometry_implementation _post_processing_geometry(std::move(_geometry_data));
            shader _post_processing_vertex_shader(shader_data { post_processing_vertex });
            shader _post_processing_fragment_shader(shader_data { post_processing_fragment });

            _persistent_post_processing_mesh.emplace(_post_processing_geometry);
            _persistent_post_processing_program = program_implementation(_post_processing_vertex_shader, _post_processing_fragment_shader);
            _is_post_processing_setup = true;
        }

        mesh_implementation& _post_processing_mesh = _persistent_post_processing_mesh.value();
        program_implementation& _post_processing_program = _persistent_post_processing_program.value();

        framebuffer_implementation::use_default();

        _post_processing_program.use();
        _post_processing_program.bind_attribute("vert_position", _post_processing_mesh, mesh_attribute::position);
        _post_processing_program.bind_uniform("uniform_color", scene_color_texture.value(), 0);
        _post_processing_program.bind_uniform("uniform_texel_size", 1.f / glm::vec2(engine_window().screen_size));

        // fxaa
        _post_processing_program.bind_uniform("uniform_fxaa_enable", fxaa_enable ? 1.f : 0.f);
        _post_processing_program.bind_uniform("uniform_fxaa_contrast_threshold", fxaa_contrast_threshold);
        _post_processing_program.bind_uniform("uniform_fxaa_relative_threshold", fxaa_relative_threshold);
        _post_processing_program.bind_uniform("uniform_fxaa_edge_sharpness", fxaa_edge_sharpness);

        _post_processing_program.draw(false);
    }

    void rendering_system::draw_debug_guizmos()
    {
#if defined(LUCARIA_DEBUG)
        _dynamics_world->setDebugDrawer(&guizmo_draw);
        _dynamics_world->debugDrawWorld();

        // show/hide from key
        if (!last_show_physics_guizmos_key && get_buttons()[input_key::keyboard_o].state) {
            show_physics_guizmos = !show_physics_guizmos;
        }
        last_show_physics_guizmos_key = get_buttons()[input_key::keyboard_o].state;

        // draw guizmos
        if (show_physics_guizmos) {
            for (const std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : guizmo_draw.positions) {
                const glm::vec3& _color = _pair.first;
                const std::vector<glm::vec3>& _positions = _pair.second;
                const std::vector<glm::uvec2>& _indices = guizmo_draw.indices.at(_color);
                if (guizmo_meshes.find(_color) == guizmo_meshes.end()) {
                    guizmo_meshes.emplace(_color, _detail::guizmo_mesh(_positions, _indices));
                } else {
                    guizmo_meshes.at(_color).update(_positions, _indices);
                }
            }
            static bool _is_program_setup = false;
            static std::optional<program_implementation> _persistent_guizmo_program = std::nullopt;
            if (!_is_program_setup) {
                shader _guizmo_vertex_shader(shader_data { guizmo_vertex });
                shader _guizmo_fragment_shader(shader_data { guizmo_fragment });
                _persistent_guizmo_program = program_implementation(_guizmo_vertex_shader, _guizmo_fragment_shader);
                _is_program_setup = true;
            }
            program_implementation& _guizmo_program = _persistent_guizmo_program.value();
            _guizmo_program.use();
            for (const std::pair<const glm::vec3, _detail::guizmo_mesh>& _pair : guizmo_meshes) {
                _guizmo_program.bind_guizmo("vert_position", _pair.second);
                _guizmo_program.bind_uniform("uniform_color", _pair.first);
                _guizmo_program.bind_uniform("uniform_mvp", camera_view_projection);
                _guizmo_program.draw_guizmo();
            }
        }

        // clear guizmos
        for (std::pair<const glm::vec3, std::vector<glm::vec3>>& _pair : guizmo_draw.positions) {
            const glm::vec3 _color = _pair.first;
            _pair.second.clear();
            guizmo_draw.indices.at(_color).clear();
        }
#endif
    }
}

void rendering_context::use_skybox_cubemap(const cubemap_object cubemap)
{
    detail::skybox_cubemap = cubemap;
}

void rendering_context::set_skybox_rotation(const float32 rotation)
{
    detail::_skybox_rotation = rotation;
}

void rendering_context::use_camera_transform(transform_component& camera)
{
    detail::_follow = &camera;
}

void rendering_context::use_camera_bone(animator_component& animator, const std::string& bone)
{
    detail::_follow_animator = &animator;
    detail::_follow_bone_name = bone;
}

void rendering_context::set_camera_fov(const glm::float32 fov)
{
    detail::camera_fov = fov;
}

void rendering_context::set_camera_near(const glm::float32 near)
{
    detail::camera_near = near;
}

void rendering_context::set_camera_far(const glm::float32 far)
{
    detail::camera_far = far;
}

void rendering_context::set_camera_rotation(const glm::float32 yaw, const glm::float32 pitch)
{
    detail::_camera_yaw += yaw;
    detail::_camera_pitch += pitch;
}

void rendering_context::set_clear_color(const glm::vec4& color)
{
    detail::clear_color = color;
}

void rendering_context::set_clear_depth(const bool is_clearing)
{
    detail::clear_depth = is_clearing;
}

void rendering_context::set_fxaa_enable(const bool enable)
{
    detail::fxaa_enable = enable;
}

}