#pragma once

#include <core/navmesh.hpp>
#include <core/fetch.hpp>

enum struct collider_algorithm {
    wall,
    ground
};

template <collider_algorithm algorithm_t>
struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other);
    collider_component& operator=(collider_component&& other);
    ~collider_component();

    collider_component& navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh);

private:
    bool _is_instanced = false;
    std::optional<std::shared_future<std::shared_ptr<navmesh_ref>>> _fetched_navmesh = std::nullopt;
    std::shared_ptr<navmesh_ref> _navmesh = nullptr;
    fetch_container<navmesh_ref> _lol = {};
    btDefaultMotionState* _state = nullptr;
    btRigidBody* _rigidbody = nullptr;
    friend struct dynamics_system;
};