#pragma once

#include <lucaria/core/cubemap.hpp>
#include <lucaria/ecs/component/transform.hpp>

namespace lucaria {
namespace ecs {

    namespace rendering {

        void use_skybox_cubemap(cubemap& from);
        void use_skybox_cubemap(fetched<cubemap>& from);
        void use_camera_transform(transform_component& camera);
        void use_camera_bone(animator_component& animator, const std::string& bone);

        void set_camera_projection(const float fov = 60.f, const float near = 0.1f, const float far = 100.f);
        void set_clear_color(const glm::vec4& color = { 1.f, 1.f, 1.f, 1.f });
        void set_clear_depth(const bool is_clearing = true);

        [[nodiscard]] glm::mat4 get_projection();
        [[nodiscard]] glm::mat4 get_view();
    }
}

namespace detail {

    struct rendering_system {
        rendering_system() = delete;
        rendering_system(const rendering_system& other) = delete;
        rendering_system& operator=(const rendering_system& other) = delete;
        rendering_system(rendering_system&& other) = delete;
        rendering_system& operator=(rendering_system&& other) = delete;

        static void clear_screen();
        static void clear_debug_guizmos();
        static void compute_projection();
        static void compute_view_projection();
        static void draw_skybox();
        static void draw_blockout_meshes();
        static void draw_unlit_meshes();
        static void draw_imgui_screen_interfaces();
        static void draw_imgui_spatial_interfaces();
        static void draw_debug_guizmos();
    };

}
}
