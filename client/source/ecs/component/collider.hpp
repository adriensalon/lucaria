#pragma once

#include <core/fetch.hpp>
#include <core/layer.hpp>
#include <core/navmesh.hpp>

struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other);
    collider_component& operator=(collider_component&& other);
    ~collider_component();

    collider_component& navmesh(const std::shared_future<std::shared_ptr<navmesh_ref>>& fetched_navmesh);
    collider_component& ground();
    collider_component& wall();
    collider_component& layer(const kinematic_layer layer);

private:
    bool _is_instanced = false;
    fetch_container<navmesh_ref> _navmesh = {};
    btDefaultMotionState* _state = nullptr;
    btRigidBody* _rigidbody = nullptr;
    short _group = 0;
    short _mask = bulletgroupID_kinematic_rigidbody;
    friend struct dynamics_system;
};