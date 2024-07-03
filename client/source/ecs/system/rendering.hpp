#pragma once

#include <future>

#include <glue/update.hpp>
#include <core/cubemap.hpp>
#include <core/program.hpp>

/// @brief 
struct rendering_system {
    REGISTER_FOR_UPDATE(rendering_system)

    rendering_system() = delete;
    rendering_system(const rendering_system& other) = delete;
    rendering_system& operator=(const rendering_system& other) = delete;
    rendering_system(rendering_system&& other) = delete;
    rendering_system& operator=(rendering_system&& other) = delete;

    /// @brief
    /// @param fov
    /// @param near
    /// @param far
    static void camera_projection(const float fov = 60.f, const float near = 0.1f, const float far = 100.f);

    /// @brief
    /// @param color
    static void clear_color(const glm::vec4& color);

    /// @brief
    /// @param clear
    static void clear_depth(const bool clear);

    /// @brief 
    /// @param cubemap
    static void cubemap_skybox(std::future<cubemap_data>&& cubemap);

    /// @brief 
    static void update();

    /// @brief 
    /// @return 
    static glm::mat4x4 get_projection();
};