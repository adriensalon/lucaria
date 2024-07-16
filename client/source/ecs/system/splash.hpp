#pragma once

#include <future>

#include <core/texture.hpp>

struct splash_system {
    splash_system() = delete;
    splash_system(const splash_system& other) = delete;
    splash_system& operator=(const splash_system& other) = delete;
    splash_system(splash_system&& other) = delete;
    splash_system& operator=(splash_system&& other) = delete;

    static bool is_splash_on();
    static void splash_texture(std::future<texture_data>&& texture);
    static void trigger_splash(const bool titlescreen);

    static void update();
};