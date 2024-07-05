#include <ecs/component/model.hpp>
#include <glue/fetch.hpp>

model_component& model_component::mesh(mesh_ref&& value)
{
    _mesh = std::move(value);
    return *this;
}

model_component& model_component::mesh(std::future<mesh_ref>&& value)
{
    _future_mesh = std::move(value);
    _mesh = std::nullopt;
    return *this;
}

model_component& model_component::texture(const model_texture type, texture_ref&& value)
{
    _textures.insert_or_assign(type, std::move(value));
    return *this;
}

model_component& model_component::texture(const model_texture type, std::future<texture_ref>&& value)
{
    _future_textures.insert_or_assign(type, std::move(value));
    _textures.insert_or_assign(type, std::nullopt);
    return *this;
}
