#pragma once

#include <future>

#include <core/volume.hpp>

struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other) = default;
    collider_component& operator=(collider_component&& other) = default;

    /// @brief 
    /// @param volume 
    collider_component& volume(std::future<volume_data>&& volume);

private:
    std::optional<std::reference_wrapper<std::future<volume_data>>> _future_volume = std::nullopt;
    std::optional<volume_data> _volume = std::nullopt;
};