#pragma once

#include <btBulletDynamicsCommon.h>

#include <core/mesh.hpp>

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
#if LUCARIA_GUIZMO
    std::unique_ptr<guizmo_mesh_ref> _guizmo;
    friend struct rendering_system;
#endif
};

std::shared_future<std::shared_ptr<navmesh_ref>> fetch_navmesh(const std::filesystem::path& mesh_path);
