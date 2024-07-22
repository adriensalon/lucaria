#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include <core/mesh.hpp>

enum struct shape_type {
    box,
    sphere,
    capsule,
    cylinder,
    cone,
    convex_hull,
    impact_triangle_mesh
};

template <shape_type shape_t>
struct shape_data;

template <>
struct shape_data<shape_type::box> {
    glm::vec3 half_extents;
};

template <>
struct shape_data<shape_type::sphere> {
    glm::float32 radius;
};

template <>
struct shape_data<shape_type::capsule> {
    // todo
};

template <>
struct shape_data<shape_type::cylinder> {
    // todo
};

template <>
struct shape_data<shape_type::cone> {
    // todo
};

struct shape_ref {
    shape_ref() = delete;
    shape_ref(const shape_ref& other) = delete;
    shape_ref& operator=(const shape_ref& other) = delete;
    shape_ref(shape_ref&& other);
    shape_ref& operator=(shape_ref&& other);
    ~shape_ref();

    template <shape_type shape_t = shape_type::box> 
    shape_ref(const shape_data<shape_t>& data);
    
    template <shape_type shape_t = shape_type::convex_hull> 
    shape_ref(const geometry_data& data);
    
    btCollisionShape* get_shape() const;

private:
    bool _is_instanced;
    btCollisionShape* _shape;
};

std::shared_future<std::shared_ptr<shape_ref>> fetch_shape(const std::filesystem::path& mesh_path);
