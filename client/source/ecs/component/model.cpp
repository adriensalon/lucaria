#include <ecs/component/model.hpp>
#include <core/fetch.hpp>

// template <model_shader shader_t>
// model_component<shader_t>& model_component<shader_t>::material(const std::shared_future<std::shared_ptr<material_ref>>& fetched_material)
// {
//     _material.emplace(fetched_material);
//     return *this;
// }

// template <model_shader shader_t>
// model_component<shader_t>& model_component<shader_t>::mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh)
// {
//     _mesh.emplace(fetched_mesh);
//     return *this;
// }

model_component<model_shader::unlit>& model_component<model_shader::unlit>::color(const std::shared_future<std::shared_ptr<texture_ref>>& fetched_color)
{
    _color.emplace(fetched_color);
    return *this;
}

model_component<model_shader::unlit>& model_component<model_shader::unlit>::mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh)
{
    _mesh.emplace(fetched_mesh);
    return *this;
}

model_component<model_shader::blockout>& model_component<model_shader::blockout>::mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh)
{
    _mesh.emplace(fetched_mesh);
    return *this;
}

// template struct model_component<model_shader::unlit>;
// template struct model_component<model_shader::pbr>;