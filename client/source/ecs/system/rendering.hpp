#pragma once

#include <core/cubemap.hpp>

struct rendering_system {
    rendering_system() = delete;
    rendering_system(const rendering_system& other) = delete;
    rendering_system& operator=(const rendering_system& other) = delete;
    rendering_system(rendering_system&& other) = delete;
    rendering_system& operator=(rendering_system&& other) = delete;

    static void camera_projection(const float fov = 60.f, const float near = 0.1f, const float far = 100.f);
    static void clear_color(const glm::vec4& color);
    static void clear_depth(const bool clear);
    static void cubemap_skybox(std::future<cubemap_data>&& cubemap);
    static glm::mat4x4 get_projection();

    static void update();
};