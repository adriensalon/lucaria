#pragma once

#include <chrono>
#include <future>

#include <core/texture.hpp>

/// @brief 
struct splash_system {

    /// @brief 
    splash_system() = delete;

    /// @brief 
    /// @param other 
    splash_system(const splash_system& other) = delete;

    /// @brief 
    /// @param other 
    /// @return 
    splash_system& operator=(const splash_system& other) = delete;
    
    /// @brief 
    /// @param other 
    splash_system(splash_system&& other) = delete;

    /// @brief 
    /// @param other 
    /// @return 
    splash_system& operator=(splash_system&& other) = delete;

    /// @brief 
    static void splash_texture(std::future<texture_data>&& texture);

    /// @brief 
    static void splash_duration(const std::chrono::seconds& duration);

    /// @brief 
    static void update();

};