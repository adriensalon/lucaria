#pragma once

#include <btBulletDynamicsCommon.h>

#include <core/mesh.hpp>

// enum struct shape_kind {
//     static,
//     dynamic
// };

enum struct static_shape_kind {
    box,
    sphere,
    capsule,
    cylinder,
    cone,
    convex_hull,
    impact_triangle_mesh
};

enum struct dynamic_shape_kind {
    box,
    sphere,
    capsule,
    cylinder,
    cone,
    
};

struct navmesh_ref {
    navmesh_ref() = delete;
    navmesh_ref(const navmesh_ref& other) = delete;
    navmesh_ref& operator=(const navmesh_ref& other) = delete;
    navmesh_ref(navmesh_ref&& other);
    navmesh_ref& operator=(navmesh_ref&& other);
    ~navmesh_ref();

    navmesh_ref(const mesh_data& data);
    btCollisionShape* get_shape() const;

private:
    bool _is_instanced;
    btCollisionShape* _shape;
};

std::shared_future<std::shared_ptr<navmesh_ref>> fetch_navmesh(const std::filesystem::path& mesh_path);
