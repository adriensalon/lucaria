#pragma once

#include <future>

#include <core/volume.hpp>

struct collider_component {

    /// @brief 
    collider_component() = delete;

    /// @brief 
    /// @param volume 
    collider_component(const volume_data& volume);

    /// @brief 
    /// @param volume 
    collider_component(std::future<volume_data>&& volume);

    /// @brief 
    /// @param other 
    collider_component(const collider_component& other) = delete;
    
    /// @brief 
    /// @param other 
    /// @return 
    collider_component& operator=(const collider_component& other) = delete;

    /// @brief 
    /// @param other 
    collider_component(collider_component&& other) = default;

    /// @brief 
    /// @param other 
    /// @return 
    collider_component& operator=(collider_component&& other) = default;

private:
    std::optional<std::reference_wrapper<std::future<volume_data>>> _future_volume = std::nullopt;
    std::optional<volume_data> _volume = std::nullopt;
};