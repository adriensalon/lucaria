#include <iostream>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <backends/imgui_impl_opengl3.h>
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#include <backends/imgui_impl_vulkan.h>
#include <lucaria/core/rendering_vulkan.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <backends/imgui_impl_psp.h>
#endif

#include <lucaria/core/systems_dynamics.hpp>
#include <lucaria/core/systems_rendering.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_transform.hpp>

#if defined(LUCARIA_BACKEND_VULKAN)
#include <glm/ext/matrix_clip_space.hpp>
#endif

namespace lucaria {
namespace detail {

	system_rendering::system_rendering()
	{
		asset_mesh::meshes_registry = &meshes_registry;
		asset_texture::textures_registry = &textures_registry;
	}

    void system_rendering::clear_runtime_references_for_reload()
    {
        // These pointers/handles can reference components or asset cells that are
        // destroyed and reconstructed during hot reload. They are configured from
        // context_rendering, so they must not survive a snapshot reload unless a
        // scene explicitly rebinds them later.
        _follow = nullptr;
        _follow_animator = nullptr;
    }

    namespace {

#if defined(LUCARIA_BACKEND_VULKAN)
        static const std::string unlit_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(location = 1) in vec2 vert_texcoord;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec2 frag_texcoord;
    void main() {
        frag_texcoord = vert_texcoord;
        gl_Position = uniform_matrix * vec4(vert_position, 1);
    })";

        static const std::string unlit_skinned_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(location = 1) in vec2 vert_texcoord;
    layout(location = 3) in ivec4 vert_bones;
    layout(location = 4) in vec4 vert_weights;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(set = 1, binding = 0) uniform lucaria_bones {
        mat4 uniform_bones_transforms[100];
        mat4 uniform_bones_invposes[100];
    };
    layout(location = 0) out vec2 frag_texcoord;
    void main() {
        frag_texcoord = vert_texcoord;
        vec4 skinned_position = vec4(0.0);
        for (int i = 0; i < 4; ++i) {
            if (vert_weights[i] > 0.0) {
                int _index = vert_bones[i];
                skinned_position += vert_weights[i] * uniform_bones_transforms[_index] * uniform_bones_invposes[_index] * vec4(vert_position, 1.0);
            }
        }
        gl_Position = uniform_matrix * skinned_position;
    })";

        static const std::string unlit_fragment = R"(#version 450
    layout(location = 0) in vec2 frag_texcoord;
    layout(set = 0, binding = 0) uniform sampler2D uniform_color;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color_tint;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec4 output_color;
    void main() {
        vec2 _atlas_texcoord = uniform_color_uv_rect.xy + frag_texcoord * uniform_color_uv_rect.zw;
        output_color = texture(uniform_color, _atlas_texcoord) * uniform_color_tint;
    })";

        static const std::string blockout_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(location = 2) in vec3 vert_normal;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec3 frag_position;
    layout(location = 1) out vec3 frag_normal;
    layout(location = 2) out vec3 uv_x;
    layout(location = 3) out vec3 uv_y;
    layout(location = 4) out vec3 uv_z;
    void main() {
        frag_position = vert_position;
        frag_normal = normalize(vert_normal);
        vec3 abs_normal = abs(frag_normal);
        vec3 uv = frag_position;
        uv_x = vec3(uv.y, uv.z, abs_normal.x);
        uv_y = vec3(uv.x, uv.z, abs_normal.y);
        uv_z = vec3(uv.x, uv.y, abs_normal.z);
        gl_Position = uniform_matrix * vec4(vert_position, 1.0);
    })";

        static const std::string blockout_fragment = R"(#version 450
    layout(location = 0) in vec3 frag_position;
    layout(location = 1) in vec3 frag_normal;
    layout(location = 2) in vec3 uv_x;
    layout(location = 3) in vec3 uv_y;
    layout(location = 4) in vec3 uv_z;
    layout(location = 0) out vec4 output_color;
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
        vec3 base_color = vec3(0.8);
        vec3 line_color = vec3(0.5);
        vec3 final_color = mix(base_color, line_color, grid_line);
        output_color = vec4(final_color, 1.0);
    })";

        static const std::string pbr_vertex = R"(#version 450
    )";

        static const std::string pbr_skinned_vertex = R"(#version 450
    )";

        static const std::string pbr_fragment = R"(#version 450
    )";

        static const std::string skybox_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec3 frag_texcoord;
    void main() {
        frag_texcoord = vert_position;
        vec4 projected = uniform_matrix * vec4(vert_position, 1);
        gl_Position = vec4(projected.xy, projected.w, projected.w);
    })";

        static const std::string skybox_fragment = R"(#version 450
    layout(location = 0) in vec3 frag_texcoord;
    layout(set = 0, binding = 0) uniform samplerCube uniform_color;
    layout(location = 0) out vec4 output_color;
    void main() {
        output_color = texture(uniform_color, frag_texcoord);
    })";

#if defined(LUCARIA_DEBUG)
        static const std::string guizmo_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    void main() {
        gl_Position = uniform_matrix * vec4(vert_position, 1.0);
    })";

        static const std::string guizmo_fragment = R"(#version 450
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec4 output_color;
    void main() {
        output_color = vec4(uniform_color.xyz, 1.0);
    })";
#endif

        static const std::string post_processing_vertex = R"(#version 450
    layout(location = 0) in vec3 vert_position;
    layout(location = 0) out vec2 frag_texcoord;
    void main() {
        gl_Position = vec4(vert_position, 1.0);
        frag_texcoord = vert_position.xy * 0.5 + 0.5;
    })";

        static const std::string post_processing_fragment = R"(#version 450
    layout(location = 0) in vec2 frag_texcoord;
    layout(set = 0, binding = 0) uniform sampler2D uniform_color;
    layout(push_constant) uniform lucaria_push {
        mat4 uniform_matrix;
        vec4 uniform_color_tint;
        vec4 uniform_color_uv_rect;
        vec4 uniform_texel_size_fxaa;
        vec4 uniform_fxaa_params;
    };
    layout(location = 0) out vec4 output_color;

    float fxaa_luma(vec3 c) {
        return dot(c, vec3(0.299, 0.587, 0.114));
    }

    void main()
    {
        vec2 uniform_texel_size = uniform_texel_size_fxaa.xy;
        float uniform_fxaa_enable = uniform_texel_size_fxaa.z;
        float uniform_fxaa_contrast_threshold = uniform_fxaa_params.x;
        float uniform_fxaa_relative_threshold = uniform_fxaa_params.y;
        float uniform_fxaa_edge_sharpness = uniform_fxaa_params.z;
        vec2 _texcoord = uniform_color_uv_rect.xy + frag_texcoord * uniform_color_uv_rect.zw;
        vec3 _rgb_m = texture(uniform_color, _texcoord).rgb;
        float _luma_m = fxaa_luma(_rgb_m);
        vec2 _texel_size = uniform_texel_size * abs(uniform_color_uv_rect.zw);
        vec3 _rgb_n = texture(uniform_color, _texcoord + vec2(0.0, -_texel_size.y)).rgb;
        vec3 _rgb_s = texture(uniform_color, _texcoord + vec2(0.0, _texel_size.y)).rgb;
        vec3 _rgb_w = texture(uniform_color, _texcoord + vec2(-_texel_size.x, 0.0)).rgb;
        vec3 _rgb_e = texture(uniform_color, _texcoord + vec2( _texel_size.x, 0.0)).rgb;
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
        _direction *= _texel_size * uniform_fxaa_edge_sharpness;
        vec3 _rgb_a = texture(uniform_color, _texcoord + _direction * 0.5).rgb;
        vec3 _rgb_b = texture(uniform_color, _texcoord - _direction * 0.5).rgb;
        vec3 _aa_color = (_rgb_a + _rgb_b + _rgb_m) / 3.0;
        vec3 _final_color = mix(_rgb_m, _aa_color, uniform_fxaa_enable);
        output_color = vec4(_final_color, 1.0);
    })";
#else
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
    uniform vec4 uniform_color_uv_rect;
    out vec4 output_color;
    void main() {
        vec2 _atlas_texcoord = uniform_color_uv_rect.xy + frag_texcoord * uniform_color_uv_rect.zw;
        output_color = texture(uniform_color, _atlas_texcoord);
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

#if defined(LUCARIA_BACKEND_PSPGU)
        [[nodiscard]] static data_geometry _make_skybox_face_geometry(const uint32 face)
        {
            static const std::array<std::array<float32x3, 4>, 6> _positions = {
                std::array<float32x3, 4> { float32x3(1.f, -1.f, -1.f), float32x3(1.f, -1.f, 1.f), float32x3(1.f, 1.f, 1.f), float32x3(1.f, 1.f, -1.f) },
                std::array<float32x3, 4> { float32x3(-1.f, 1.f, -1.f), float32x3(1.f, 1.f, -1.f), float32x3(1.f, 1.f, 1.f), float32x3(-1.f, 1.f, 1.f) },
                std::array<float32x3, 4> { float32x3(1.f, -1.f, 1.f), float32x3(-1.f, -1.f, 1.f), float32x3(-1.f, 1.f, 1.f), float32x3(1.f, 1.f, 1.f) },
                std::array<float32x3, 4> { float32x3(-1.f, -1.f, 1.f), float32x3(-1.f, -1.f, -1.f), float32x3(-1.f, 1.f, -1.f), float32x3(-1.f, 1.f, 1.f) },
                std::array<float32x3, 4> { float32x3(-1.f, -1.f, 1.f), float32x3(1.f, -1.f, 1.f), float32x3(1.f, -1.f, -1.f), float32x3(-1.f, -1.f, -1.f) },
                std::array<float32x3, 4> { float32x3(-1.f, -1.f, -1.f), float32x3(1.f, -1.f, -1.f), float32x3(1.f, 1.f, -1.f), float32x3(-1.f, 1.f, -1.f) },
            };

            data_geometry _geometry = {};
            _geometry.positions.assign(_positions[face].begin(), _positions[face].end());
            _geometry.texcoords = {
                float32x2(0.f, 1.f),
                float32x2(1.f, 1.f),
                float32x2(1.f, 0.f),
                float32x2(0.f, 0.f)
            };
            _geometry.indices = {
                uint32x3(0, 1, 2),
                uint32x3(0, 2, 3)
            };
            return _geometry;
        }
#endif

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
#endif

        struct raycast_data {
            float32x3 origin;
            float32x3 direction;
        };

        static float32x3 forward_from_yaw_pitch(float yaw_deg, float pitch_deg)
        {
            float y = glm::radians(yaw_deg), p = glm::radians(pitch_deg);
            float cp = cos(p), sp = sin(p), cy = cos(y), sy = sin(y);
            return glm::normalize(float32x3(cp * sy, sp, -cp * cy));
        }

        static void camera_basis_from_forward(const float32x3& fwd, const float32x3& upRef, float32x3& right, float32x3& up)
        {
            right = glm::normalize(glm::cross(fwd, upRef));
            up = glm::normalize(glm::cross(right, fwd));
        }

        [[nodiscard]] static bool viewport_raycast_triangle(
            const raycast_data& raycast,
            const float32x3& vertex_position_a,
            const float32x3& vertex_position_b,
            const float32x3& vertex_position_c,
            float32x3& collision_position)
        {
            const float32 _eps = 1e-7f;
            const float32x3 _ab = vertex_position_b - vertex_position_a;
            const float32x3 _ac = vertex_position_c - vertex_position_a;

            const float32x3 _p = glm::cross(raycast.direction, _ac);
            const float32 _det = glm::dot(_ab, _p);
            if (_det < _eps) {
                return false; // we cull backfaces
            }

            const float32 _inv_det = 1.f / _det;
            const float32x3 _s = raycast.origin - vertex_position_a;
            collision_position.y = glm::dot(_s, _p) * _inv_det;
            if (collision_position.y < 0.f || collision_position.y > 1.f) {
                return false;
            }

            const float32x3 _q = glm::cross(_s, _ab);
            collision_position.z = glm::dot(raycast.direction, _q) * _inv_det;
            if (collision_position.z < 0.f || collision_position.x + collision_position.z > 1.f) {
                return false;
            }

            collision_position.x = glm::dot(_ac, _q) * _inv_det;
            return collision_position.x >= 0.f;
        }

        [[nodiscard]] static float32x2 viewport_lerp_uv(
            const float32x2& vertex_texcoord_a,
            const float32x2& vertex_texcoord_b,
            const float32x2& vertex_texcoord_c,
            const float32 lerp_texcoord_u,
            const float32 lerp_texcoord_v)
        {
            const float32 _w = 1.0f - lerp_texcoord_u - lerp_texcoord_v;
            return vertex_texcoord_a * _w + vertex_texcoord_b * lerp_texcoord_u + vertex_texcoord_c * lerp_texcoord_v;
        }

        [[nodiscard]] std::optional<float32x2> viewport_raycast(const float32x4x4& camera_view, const asset_geometry& viewport_geometry)
        {
            float32x4x4 _inverse_view = glm::inverse(camera_view);
            float32x3 _origin = float32x3(_inverse_view * glm::vec4(0, 0, 0, 1));
            float32x3 _direction = glm::normalize(float32x3(_inverse_view * glm::vec4(0, 0, -1, 0)));
            const raycast_data _raycast { _origin, _direction };
            bool _has_hit = false;
            float32 _best_distance = std::numeric_limits<float32>::infinity();
            float32x2 _best_uv = float32x2(0);

            // compute for each triangle
            for (const uint32x3& _triangle : viewport_geometry.data.indices) {
                const float32x3& _vertex_a = viewport_geometry.data.positions[_triangle.x];
                const float32x3& _vertex_b = viewport_geometry.data.positions[_triangle.y];
                const float32x3& _vertex_c = viewport_geometry.data.positions[_triangle.z];
                float32x3 _collision_position;
                if (!viewport_raycast_triangle(_raycast, _vertex_a, _vertex_b, _vertex_c, _collision_position)) {
                    continue;
                }
                if (_collision_position.x < _best_distance) {
                    const float32x2& _texcoord_a = viewport_geometry.data.texcoords[_triangle.x];
                    const float32x2& _texcoord_b = viewport_geometry.data.texcoords[_triangle.y];
                    const float32x2& _texcoord_c = viewport_geometry.data.texcoords[_triangle.z];
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

        [[nodiscard]] static float32quat extract_world_rotation(const float32x4x4& transform)
        {
            float32x3x3 rotation = float32x3x3(transform);
            rotation[0] = glm::normalize(rotation[0]);
            rotation[2] = glm::normalize(glm::cross(rotation[0], glm::normalize(rotation[1])));
            rotation[1] = glm::normalize(glm::cross(rotation[2], rotation[0]));
            return glm::quat_cast(float32x4x4(rotation));
        }
    }


    void system_rendering::use_camera_transform(handle_entity entity)
    {
        _follow_entity = entity;
        _follow_bone_name.clear();
        _follow = nullptr;
        _follow_animator = nullptr;
    }

    void system_rendering::use_camera_bone(handle_entity entity, std::string bone)
    {
        _follow_entity = entity;
        _follow_bone_name = std::move(bone);
        _follow = nullptr;
        _follow_animator = nullptr;
    }

    void system_rendering::resolve_runtime_references(manager_scenes& scenes)
    {
        _follow = nullptr;
        _follow_animator = nullptr;

        if (!_follow_entity || !_follow_entity->has_value()) {
            return;
        }

        _follow = scenes.registry.try_get<component_transform>(_follow_entity->_entity);
        if (!_follow_bone_name.empty()) {
            _follow_animator = scenes.registry.try_get<component_animator>(_follow_entity->_entity);
        }
    }

    void system_rendering::update_clear_screen(manager_window& window, manager_scenes& scenes)
    {
#if defined(LUCARIA_BACKEND_PSPGU)
        rendering_framebuffer::use_default();
        rendering_program::viewport(window.screen_size);
        rendering_program::clear(true);
        return;
#endif

        if (!scene_framebuffer) {
            scene_framebuffer.emplace();
        }
        if (!scene_color_texture) {
            scene_color_texture.emplace(window.screen_size);
            scene_framebuffer->bind_color(scene_color_texture.value().texture);
        } else {
            scene_color_texture->resize(window.screen_size);
        }
        if (!scene_depth_renderbuffer) {
#if defined(LUCARIA_BACKEND_OPENGL)
            scene_depth_renderbuffer = rendering_renderbuffer(window.screen_size, GL_DEPTH_COMPONENT24);
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
            scene_depth_renderbuffer = rendering_renderbuffer(window.screen_size, VK_FORMAT_D32_SFLOAT);
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
            // scene_depth_renderbuffer = rendering_renderbuffer(window.screen_size, GL_DEPTH_COMPONENT24); TODO
#endif
            scene_framebuffer->bind_depth(scene_depth_renderbuffer.value());
        } else {
            scene_depth_renderbuffer->resize(window.screen_size);
        }

        scene_framebuffer->use();
        rendering_program::viewport(window.screen_size);
        rendering_program::clear(true);
    }

    void system_rendering::update_compute_projection(manager_window& window, manager_scenes& scenes)
    {
        float32x2 _screen_size = window.screen_size;
        float _fov_rad = glm::radians(camera_fov);
        float _aspect_ratio = _screen_size.x / _screen_size.y;
#if defined(LUCARIA_BACKEND_VULKAN)
        camera_projection = glm::perspectiveRH_ZO(_fov_rad, _aspect_ratio, camera_far, camera_near);
#else
        camera_projection = glm::perspective(_fov_rad, _aspect_ratio, camera_near, camera_far);
#endif
    }

    void system_rendering::update_apply_camera_rotation(manager_scenes& scenes)
    {
        resolve_runtime_references(scenes);
        if (!_follow) {
            return;
        }

        const float32x2 _mouse_delta = float32x2(_camera_yaw, _camera_pitch);
        if (_mouse_delta == float32x2(0.f)) {
            return;
        }

        const float _yaw_delta_degrees = -_mouse_delta.x * mouse_sensitivity;
        player_pitch -= _mouse_delta.y * mouse_sensitivity;
        player_pitch = glm::clamp(player_pitch, -89.f, 89.f);
        _camera_yaw = 0.f;
        _camera_pitch = 0.f;

        const float32x4x4 followW0 = _follow->_transform;
        const float32quat qFollow0 = extract_world_rotation(followW0);
        const float32x3 charUp = glm::normalize(qFollow0 * float32x3(0, 1, 0));
        const float32quat qYawDelta = glm::angleAxis(glm::radians(_yaw_delta_degrees), charUp);
        const float32quat qFollow1 = glm::normalize(qYawDelta * qFollow0);

        float32x4x4 followW1 = glm::mat4_cast(qFollow1);
        followW1[3] = followW0[3];
        _follow->set_transform_warp(followW1);
    }

    void system_rendering::update_compute_view_projection(manager_scenes& scenes)
    {
        resolve_runtime_references(scenes);
        if (_follow && _follow_animator && !_follow_bone_name.empty()) {
            const float32x4x4 followW = _follow->_transform;
            const float32quat qFollow = extract_world_rotation(followW);
            const float32x3 camF_local = forward_from_yaw_pitch(0.f, player_pitch);
            const float32x3 flatF_local = forward_from_yaw_pitch(0.f, 0.f);
            const float32x3 camForward = glm::normalize(qFollow * camF_local);
            const float32x3 groundF = glm::normalize(qFollow * flatF_local);

            const float32x3 worldUp = { 0, 1, 0 };
            float32x3 camRight, camUp;
            camera_basis_from_forward(camForward, worldUp, camRight, camUp);

            const float32x4x4 boneLocal = _follow_animator->get_bone_transform(_follow_bone_name);
            const float32x3 boneWorld = float32x3((followW * boneLocal)[3]);
            // const float32x3 boneWorld = float32x3(followW[3]);

            const float boomDist = -0.23f;
            const float camHeight = 0.f;

            player_position = boneWorld - groundF * boomDist + worldUp * camHeight;
            camera_view = glm::lookAt(player_position, player_position + camForward, camUp);
        }

        camera_view_projection = camera_projection * camera_view;
    }

    void system_rendering::update_draw_skybox(manager_scenes& scenes)
    {
        if (skybox_cubemap) {
            static bool _is_skybox_setup = false;
#if defined(LUCARIA_BACKEND_PSPGU)
            static std::array<std::optional<asset_mesh>, 6> _persistent_skybox_face_meshes = {};
#else
            static std::optional<asset_mesh> _persistent_skybox_mesh = std::nullopt;
#endif
            static std::optional<rendering_program> _persistent_skybox_program = std::nullopt;

            if (!_is_skybox_setup) {
#if defined(LUCARIA_BACKEND_PSPGU)
                for (uint32 _face = 0; _face < _persistent_skybox_face_meshes.size(); ++_face) {
                    data_geometry _geometry_data = _make_skybox_face_geometry(_face);
                    asset_geometry _skybox_geometry(std::move(_geometry_data));
                    _persistent_skybox_face_meshes[_face].emplace(_skybox_geometry);
                }
#else
                data_geometry _geometry_data;
                _geometry_data.positions = skybox_positions;
                _geometry_data.indices = skybox_indices;
                asset_geometry _skybox_geometry(std::move(_geometry_data));
#endif

                object_shader _skybox_vertex_shader(data_shader { data_shader_profile::glsl, skybox_vertex });
                object_shader _skybox_fragment_shader(data_shader { data_shader_profile::glsl, skybox_fragment });

#if !defined(LUCARIA_BACKEND_PSPGU)
                _persistent_skybox_mesh.emplace(_skybox_geometry);
#endif
                _persistent_skybox_program.emplace(_skybox_vertex_shader, _skybox_fragment_shader);
                _is_skybox_setup = true;
            }

            rendering_program& _skybox_program = _persistent_skybox_program.value();
            rendering_cubemap& _skybox_cubemap = skybox_cubemap.value().cubemap;
            const float32x4x4 skybox_rotation_matrix = glm::rotate(glm::identity<float32x4x4>(), glm::radians(skybox_rotation), float32x3(0, 1, 0));
            const float32x4x4 _no_translation_view_projection = camera_projection * float32x4x4(float32x3x3(camera_view)) * skybox_rotation_matrix;
#if defined(LUCARIA_BACKEND_PSPGU)
            for (uint32 _face = 0; _face < _persistent_skybox_face_meshes.size(); ++_face) {
                if (!_persistent_skybox_face_meshes[_face].has_value() || !_skybox_cubemap.faces[_face].has_value()) {
                    continue;
                }
                rendering_mesh& _skybox_mesh = _persistent_skybox_face_meshes[_face].value().mesh;
                _skybox_program.use();
                sceGuDisable(GU_CULL_FACE);
                _skybox_program.bind_attribute("vert_position", _skybox_mesh, data_vertex_attribute::position);
                _skybox_program.bind_attribute("vert_texcoord", _skybox_mesh, data_vertex_attribute::texcoord);
                _skybox_program.bind_uniform("uniform_color", _skybox_cubemap.faces[_face].value(), 0);
                _skybox_program.bind_uniform("uniform_projection", _no_translation_view_projection);
                _skybox_program.draw(false);
            }
#else
            rendering_mesh& _skybox_mesh = _persistent_skybox_mesh.value().mesh;
            _skybox_program.use();
            _skybox_program.bind_attribute("vert_position", _skybox_mesh, data_vertex_attribute::position);
            _skybox_program.bind_uniform("uniform_color", _skybox_cubemap, 0);
            _skybox_program.bind_uniform("uniform_projection", _no_translation_view_projection);
            _skybox_program.draw(false);
#endif
        }
    }

    void system_rendering::update_draw_blockout_meshes(manager_scenes& scenes)
    {
        static bool _is_program_setup = false;
        static std::optional<rendering_program> _persistent_blockout_program = std::nullopt;
        if (!_is_program_setup) {
            object_shader _blockout_vertex_shader((data_shader { data_shader_profile::glsl, blockout_vertex }));
            object_shader _blockout_fragment_shader(data_shader { data_shader_profile::glsl, blockout_fragment });
            _persistent_blockout_program = rendering_program(_blockout_vertex_shader, _blockout_fragment_shader);
            _is_program_setup = true;
        }

        rendering_program& _blockout_program = _persistent_blockout_program.value();
        scenes.each_view<component_model_blockout, component_transform>([&](component_model_blockout& _model, component_transform& _transform) {
            if (_model._mesh) {
                const float32x4x4 _model_view_projection = camera_view_projection * _transform._transform;
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                _blockout_program.use();
                _blockout_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _blockout_program.bind_attribute("vert_normal", _mesh, data_vertex_attribute::normal);
                _blockout_program.bind_uniform("uniform_view", _model_view_projection);
                _blockout_program.draw();
            }
        });

        scenes.each_view<component_model_blockout>(exclude<component_transform>, [&](component_model_blockout& _model) {
            if (_model._mesh) {
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                _blockout_program.use();
                _blockout_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _blockout_program.bind_attribute("vert_normal", _mesh, data_vertex_attribute::normal);
                _blockout_program.bind_uniform("uniform_view", camera_view_projection);
                _blockout_program.draw();
            }
        });
    }

    void system_rendering::update_draw_unlit_meshes(manager_scenes& scenes)
    {
        static bool _is_program_setup = false;
        if (!_is_program_setup) {
            object_shader _unlit_vertex_shader(data_shader { data_shader_profile::glsl, unlit_vertex });
            object_shader _unlit_fragment_shader(data_shader { data_shader_profile::glsl, unlit_fragment });
            _persistent_unlit_program = rendering_program(_unlit_vertex_shader, _unlit_fragment_shader);
            _is_program_setup = true;
        }

        rendering_program& _unlit_program = _persistent_unlit_program.value();
        scenes.each_view<component_model_unlit, component_transform>(exclude<component_animator>, [&](component_model_unlit& _model, component_transform& _transform) {
            if (_model._mesh && _model._color) {
                const float32x4x4 _model_view_projection = camera_view_projection * _transform._transform;
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                const rendering_texture& _color = _model._color.value().texture;
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", _mesh, data_vertex_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_view", _model_view_projection);
                _unlit_program.bind_uniform("uniform_color", _color, 0);
                _unlit_program.draw();
            }
        });

        scenes.each_view<component_model_unlit>(exclude<component_transform, component_animator>, [&](component_model_unlit& _model) {
            if (_model._mesh && _model._color) {
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                const rendering_texture& _color = _model._color.value().texture;
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", _mesh, data_vertex_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_color", _color, 0);
                _unlit_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_program.draw();
            }
        });
    }

    void system_rendering::update_draw_unlit_skinned_meshes(manager_scenes& scenes)
    {
        static bool _is_program_setup = false;
        static std::optional<rendering_program> _persistent_unlit_skinned_program = std::nullopt;
        if (!_is_program_setup) {
            object_shader _unlit_fragment_shader(data_shader { data_shader_profile::glsl, unlit_fragment });
            object_shader _unlit_skinned_vertex_shader(data_shader { data_shader_profile::glsl, unlit_skinned_vertex });
            _persistent_unlit_skinned_program = rendering_program(_unlit_skinned_vertex_shader, _unlit_fragment_shader);
            _is_program_setup = true;
        }

        rendering_program& _unlit_skinned_program = _persistent_unlit_skinned_program.value();
        scenes.each_view<component_model_unlit, component_transform, component_animator>([&](component_model_unlit& _model, component_transform& _transform, component_animator& animator) {
            if (_model._mesh && _model._color && animator._skeleton.has_value()) {
                const float32x4x4 _model_view_projection = camera_view_projection * _transform._transform;
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                const rendering_texture& _color = _model._color.value().texture;
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, data_vertex_attribute::texcoord);
                _unlit_skinned_program.bind_attribute("vert_bones", _mesh, data_vertex_attribute::bones);
                _unlit_skinned_program.bind_attribute("vert_weights", _mesh, data_vertex_attribute::weights);
                _unlit_skinned_program.bind_uniform("uniform_view", _model_view_projection);
                _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.invposes);
                _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms);
                _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                _unlit_skinned_program.draw();
            }
        });

        scenes.each_view<component_model_unlit, component_animator>(exclude<component_transform>, [&](component_model_unlit& _model, component_animator& animator) {
            if (_model._mesh && _model._color && animator._skeleton.has_value()) {
                const rendering_mesh& _mesh = _model._mesh.value().mesh;
                const rendering_texture& _color = _model._color.value().texture;
                _unlit_skinned_program.use();
                _unlit_skinned_program.bind_attribute("vert_position", _mesh, data_vertex_attribute::position);
                _unlit_skinned_program.bind_attribute("vert_texcoord", _mesh, data_vertex_attribute::texcoord);
                _unlit_skinned_program.bind_attribute("vert_bones", _mesh, data_vertex_attribute::bones);
                _unlit_skinned_program.bind_attribute("vert_weights", _mesh, data_vertex_attribute::weights);
                _unlit_skinned_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_skinned_program.bind_uniform("uniform_bones_transforms[0]", animator._model_transforms);
                _unlit_skinned_program.bind_uniform("uniform_bones_invposes[0]", _mesh.invposes);
                _unlit_skinned_program.bind_uniform("uniform_color", _color, 0);
                _unlit_skinned_program.draw();
            }
        });
    }

    void system_rendering::update_draw_imgui_spatial_interfaces(manager_window& window, manager_input& input, manager_scenes& scenes)
    {
        scenes.each_view<component_interface_spatial>([this, &window, &input](component_interface_spatial& interface) {
            if (interface._viewport_geometry.has_value()
                && interface._viewport_mesh
                && interface._imgui_color_texture
                && interface._imgui_framebuffer
                && interface._imgui_callback) {

                if (interface._setup_callback) {
                    interface._setup_callback(window);
                    interface._setup_callback = nullptr;
                }

                ImGui::SetCurrentContext(interface._imgui_context);
                const uint32x2 _imgui_render_size = interface._imgui_render_size == uint32x2(0)
                    ? interface._viewport_size
                    : interface._imgui_render_size;
                ImGui::GetIO().DisplaySize = ImVec2(static_cast<float32>(_imgui_render_size.x), static_cast<float32>(_imgui_render_size.y));

                std::optional<float32x2> _raycasted_uvs;
                if (interface._use_interaction) {
                    _raycasted_uvs = viewport_raycast(camera_view, interface._viewport_geometry.value());
                    if (_raycasted_uvs) {
                        interface._interaction_screen_position = {
                            (_raycasted_uvs.value().x) * _imgui_render_size.x,
                            (1.f - _raycasted_uvs.value().y) * _imgui_render_size.y
                        };
                        ImGui::GetIO().MousePos = ImVec2(interface._interaction_screen_position.value().x, interface._interaction_screen_position.value().y);
                        ImGui::GetIO().MouseDown[0] = input.key_events[input_key::mouse_left].state;
                    } else {
                        interface._interaction_screen_position = std::nullopt;
                    }
                }

                interface._imgui_framebuffer->use();
                rendering_program::viewport(_imgui_render_size);
                rendering_program::clear(false);
#if defined(LUCARIA_BACKEND_OPENGL)
                ImGui_ImplOpenGL3_NewFrame();
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
                ImGui_ImplVulkan_NewFrame();
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
                        _drawlist->AddImage(
                            _texture_id,
                            _cursor_min,
                            _cursor_max,
                            interface._interaction_texture.imgui_uv0(),
                            interface._interaction_texture.imgui_uv1());
                    }
                }

                ImGui::SetCurrentContext(interface._imgui_context);
                ImGui::Render();
#if defined(LUCARIA_BACKEND_OPENGL)
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
                rendering_vulkan_render_imgui(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
                ImGui_ImplPSP_RenderDrawData(ImGui::GetDrawData());
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
                rendering_framebuffer::use_default();
                rendering_program::viewport(window.screen_size);
#else
                scene_framebuffer->use();
                rendering_program::viewport(window.screen_size);
#endif
                rendering_program& _unlit_program = _persistent_unlit_program.value();
                _unlit_program.use();
                _unlit_program.bind_attribute("vert_position", interface._viewport_mesh.value().mesh, data_vertex_attribute::position);
                _unlit_program.bind_attribute("vert_texcoord", interface._viewport_mesh.value().mesh, data_vertex_attribute::texcoord);
                _unlit_program.bind_uniform("uniform_color", interface._imgui_color_texture.value().texture, 0);
                _unlit_program.bind_uniform("uniform_view", camera_view_projection);
                _unlit_program.draw();
            }
        });
    }

    void system_rendering::update_draw_imgui_screen_interfaces(manager_window& window, manager_scenes& scenes)
    {
        ImGui::SetCurrentContext(window.screen_context);
#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_NewFrame();
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
        ImGui_ImplVulkan_NewFrame();
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
        ImGui_ImplPSP_NewFrame();
#endif
        ImGui::NewFrame();

        scenes.each_view<component_interface_screen>([](component_interface_screen& interface) {
            if (interface._imgui_callback) {
                interface._imgui_callback();
            }
        });

        ImGui::SetCurrentContext(window.screen_context);
        ImGui::Render();
#if defined(LUCARIA_BACKEND_OPENGL)
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
        rendering_vulkan_render_imgui(ImGui::GetDrawData());
#endif
#if defined(LUCARIA_BACKEND_PSPGU)
        ImGui_ImplPSP_RenderDrawData(ImGui::GetDrawData());
#endif
    }

    void system_rendering::update_draw_post_processing(manager_window& window, manager_scenes& scenes)
    {
#if defined(LUCARIA_BACKEND_PSPGU)
        return;
#endif

        static bool _is_post_processing_setup = false;
        static std::optional<asset_mesh> _persistent_post_processing_mesh = std::nullopt;
        static std::optional<rendering_program> _persistent_post_processing_program = std::nullopt;

        if (!_is_post_processing_setup) {
            data_geometry _geometry_data;

            _geometry_data.positions = {
                float32x3(-1.f, -1.f, 0.f),
                float32x3(1.f, -1.f, 0.f),
                float32x3(1.f, 1.f, 0.f),
                float32x3(-1.f, 1.f, 0.f),
            };

            _geometry_data.indices = {
                uint32x3(0, 1, 2),
                uint32x3(0, 2, 3),
            };

            asset_geometry _post_processing_geometry(std::move(_geometry_data));
            object_shader _post_processing_vertex_shader(data_shader { data_shader_profile::glsl, post_processing_vertex });
            object_shader _post_processing_fragment_shader(data_shader { data_shader_profile::glsl, post_processing_fragment });

            _persistent_post_processing_mesh.emplace(_post_processing_geometry);
            _persistent_post_processing_program = rendering_program(_post_processing_vertex_shader, _post_processing_fragment_shader);
            _is_post_processing_setup = true;
        }

        rendering_mesh& _post_processing_mesh = _persistent_post_processing_mesh.value().mesh;
        rendering_program& _post_processing_program = _persistent_post_processing_program.value();

        rendering_framebuffer::use_default();

        _post_processing_program.use();
        _post_processing_program.bind_attribute("vert_position", _post_processing_mesh, data_vertex_attribute::position);
        _post_processing_program.bind_uniform("uniform_color", scene_color_texture.value().texture, 0);
        _post_processing_program.bind_uniform("uniform_texel_size", 1.f / float32x2(window.screen_size));

        // fxaa
        _post_processing_program.bind_uniform("uniform_fxaa_enable", fxaa_enable ? 1.f : 0.f);
        _post_processing_program.bind_uniform("uniform_fxaa_contrast_threshold", fxaa_contrast_threshold);
        _post_processing_program.bind_uniform("uniform_fxaa_relative_threshold", fxaa_relative_threshold);
        _post_processing_program.bind_uniform("uniform_fxaa_edge_sharpness", fxaa_edge_sharpness);

        _post_processing_program.draw(false);
    }

    void system_rendering::update_draw_debug_guizmos(system_dynamics& dynamics, manager_input& input, manager_scenes& scenes)
    {
#if defined(LUCARIA_DEBUG)
        dynamics.dynamics_world->setDebugDrawer(&guizmo_draw);
        dynamics.dynamics_world->debugDrawWorld();

        // show/hide from key
        if (!last_show_physics_guizmos_key && input.key_events[input_key::keyboard_o].state) {
            show_physics_guizmos = !show_physics_guizmos;
        }
        last_show_physics_guizmos_key = input.key_events[input_key::keyboard_o].state;

        // draw guizmos
        if (show_physics_guizmos) {
            for (const std::pair<const float32x3, std::vector<float32x3>>& _pair : guizmo_draw.positions) {
                const float32x3& _color = _pair.first;
                const std::vector<float32x3>& _positions = _pair.second;
                const std::vector<uint32x2>& _indices = guizmo_draw.indices.at(_color);
                if (object_mesh_linees.find(_color) == object_mesh_linees.end()) {
                    object_mesh_linees.emplace(_color, rendering_mesh_line(_positions, _indices));
                } else {
                    object_mesh_linees.at(_color).update(_positions, _indices);
                }
            }
            static bool _is_program_setup = false;
            static std::optional<rendering_program> _persistent_guizmo_program = std::nullopt;
            if (!_is_program_setup) {
                object_shader _guizmo_vertex_shader(data_shader { data_shader_profile::glsl, guizmo_vertex });
                object_shader _guizmo_fragment_shader(data_shader { data_shader_profile::glsl, guizmo_fragment });
                _persistent_guizmo_program = rendering_program(_guizmo_vertex_shader, _guizmo_fragment_shader);
                _is_program_setup = true;
            }
            rendering_program& _guizmo_program = _persistent_guizmo_program.value();
            _guizmo_program.use();
            for (const std::pair<const float32x3, rendering_mesh_line>& _pair : object_mesh_linees) {
                _guizmo_program.bind_guizmo("vert_position", _pair.second);
                _guizmo_program.bind_uniform("uniform_color", _pair.first);
                _guizmo_program.bind_uniform("uniform_mvp", camera_view_projection);
                _guizmo_program.draw_guizmo();
            }
        }

        // clear guizmos
        for (std::pair<const float32x3, std::vector<float32x3>>& _pair : guizmo_draw.positions) {
            const float32x3 _color = _pair.first;
            _pair.second.clear();
            guizmo_draw.indices.at(_color).clear();
        }
#endif
    }

}
}
