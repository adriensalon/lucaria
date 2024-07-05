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
    return *this;
}

model_component& model_component::texture(const model_texture type, texture_ref&& value)
{
    _textures[type] = std::move(value);
    return *this;
}

model_component& model_component::texture(const model_texture type, std::future<texture_ref>&& value)
{
    _future_textures[type] = std::move(value);
    return *this;
}

void model_component::_update_futures()
{
    if (_future_mesh.has_value()) {
        std::future<mesh_ref>& _future = _future_mesh.value();
        if (get_is_future_ready(_future)) {
            _mesh = std::move(_future.get());
            _future_mesh = std::nullopt;
        }
    }
    for (std::pair<const model_texture, std::optional<std::future<texture_ref>>>& _pair : _future_textures) {
        if (_pair.second.has_value()) {
            std::future<texture_ref>& _future = _pair.second.value();
            if (get_is_future_ready(_future)) {
                _textures[_pair.first] = std::move(_future.get());
                _pair.second = std::nullopt;
            }
        }
    }
}