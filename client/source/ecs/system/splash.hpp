#pragma once

#include <future>

#include <core/texture.hpp>
#include <glue/update.hpp>

/// @brief Splash system must be drawn above any rendering system.
struct splash_system {
    splash_system() = delete;
    splash_system(const splash_system& other) = delete;
    splash_system& operator=(const splash_system& other) = delete;
    splash_system(splash_system&& other) = delete;
    splash_system& operator=(splash_system&& other) = delete;

    /// @brief Returns is the splash screen is currently showing.
    static bool is_splash_on();

    /// @brief Selects a future texture to use in the splashscreen
    static void splash_texture(std::future<texture_data>&& texture);

    /// @brief Triggers the splash screen to be drawn from now on.
    static void trigger_splash(const bool titlescreen);

    static void update();

private:
    REGISTER_FOR_UPDATE(splash_system)
};