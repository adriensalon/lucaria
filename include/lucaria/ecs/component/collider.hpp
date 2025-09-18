#pragma once

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/layer.hpp>
#include <lucaria/core/shape.hpp>
#include <lucaria/core/world.hpp>

struct collider_component {
    collider_component() = default;
    collider_component(const collider_component& other) = delete;
    collider_component& operator=(const collider_component& other) = delete;
    collider_component(collider_component&& other);
    collider_component& operator=(collider_component&& other);
    ~collider_component();

    collider_component& shape(const std::shared_future<std::shared_ptr<shape_ref>>& fetched_shape);
    collider_component& ground();
    collider_component& wall();
    collider_component& layer(const kinematic_layer layer);
    
    // collider_component& ground_callback(const std::function<void(void*)>& callback, void* data);
    // collider_component& wall_callback(const std::function<void(void*)>& callback, void* data);
    // collider_component& layer_callback(const kinematic_layer layer, const std::function<void(void*)>& callback, void* data);

private:
    bool _is_instanced = false;
    fetch_container<shape_ref> _shape = {};
    btDefaultMotionState* _state = nullptr;
    btRigidBody* _rigidbody = nullptr;
    short _group = 0;
    short _mask = bulletgroupID_kinematic_rigidbody;
    friend struct dynamics_system;
};