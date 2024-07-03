#include <ecs/component/model.hpp>

model_component::model_component(const mesh_data& mesh, const texture_data& color)
{
    _mesh = mesh_ref(mesh);
    _color = texture_ref(color);
}

model_component::model_component(std::future<mesh_data>&& mesh, std::future<texture_data>&& color)
{
    _future_mesh = std::move(mesh);
    _future_color = std::move(color);
}