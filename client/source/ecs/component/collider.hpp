#pragma once

#include <future>

#include <btBulletDynamicsCommon.h>

#include <core/volume.hpp>

enum struct collider_detection {
    passive,
    navigator
};

template <collider_detection detection_t = collider_detection::passive>
struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other) = default;
    collider_component& operator=(collider_component&& other) = default;

    collider_component& volume(const std::shared_future<std::shared_ptr<volume_ref>>& fetched_volume);

private:
    std::optional<std::shared_future<std::shared_ptr<volume_ref>>> _fetched_volume = std::nullopt;
    std::shared_ptr<volume_ref> _volume = nullptr;
    btCollisionShape* _box_shape = nullptr;
};