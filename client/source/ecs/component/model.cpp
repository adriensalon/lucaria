#include <ecs/component/model.hpp>
#include <glue/fetch.hpp>

template <model_shader shader_t>
model_component<shader_t>& model_component<shader_t>::material(const std::shared_future<std::shared_ptr<material_ref>>& fetched_material)
{
    _fetched_material = fetched_material;
    _material = nullptr;
    return *this;
}

template <model_shader shader_t>
model_component<shader_t>& model_component<shader_t>::mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh)
{
    _fetched_mesh = fetched_mesh;
    _mesh = nullptr;
    return *this;
}

template struct model_component<model_shader::unlit>;
template struct model_component<model_shader::pbr>;
template struct model_component<model_shader::blockout>;