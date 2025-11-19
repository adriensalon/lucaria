#include <lucaria/component/model.hpp>

namespace lucaria {

model_component<model_type::blockout>& model_component<model_type::blockout>::use_mesh(mesh& from)
{
    _mesh.emplace(from);
    return *this;
}

model_component<model_type::blockout>& model_component<model_type::blockout>::use_mesh(fetched<mesh>& from)
{
    _mesh.emplace(from);
    return *this;
}

model_component<model_type::unlit>& model_component<model_type::unlit>::use_mesh(mesh& from)
{
    _mesh.emplace(from);
    return *this;
}

model_component<model_type::unlit>& model_component<model_type::unlit>::use_mesh(fetched<mesh>& from)
{
    _mesh.emplace(from);
    return *this;
}

model_component<model_type::unlit>& model_component<model_type::unlit>::use_color(texture& from)
{
    _color.emplace(from);
    return *this;
}

model_component<model_type::unlit>& model_component<model_type::unlit>::use_color(fetched<texture>& from)
{
    _color.emplace(from);
    return *this;
}

}
