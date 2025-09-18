#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include <lucaria/common/geometry.hpp>

namespace lucaria {

enum struct shape_type {
    box,
    sphere,
    capsule,
    cylinder,
    cone,
    convex_hull,
    triangle_mesh,
    impact_triangle_mesh
};

struct shape_ref {
    shape_ref() = delete;
    shape_ref(const shape_ref& other) = delete;
    shape_ref& operator=(const shape_ref& other) = delete;
    shape_ref(shape_ref&& other);
    shape_ref& operator=(shape_ref&& other);
    ~shape_ref();

    shape_ref(const geometry_data& data, const shape_type shape = shape_type::convex_hull);
    btCollisionShape* get_shape() const;

private:
    bool _is_instanced;
    btCollisionShape* _shape;
};

std::shared_future<std::shared_ptr<shape_ref>> fetch_shape(const std::filesystem::path& geometry_path, const shape_type shape = shape_type::convex_hull);
void clear_shape_fetches();

}
