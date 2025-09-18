#include <lucaria/ecs/component/model.hpp>

model_component<model_shader::blockout>& model_component<model_shader::blockout>::mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh)
{
    _mesh.emplace(fetched_mesh);
    return *this;
}

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

// todo pbr