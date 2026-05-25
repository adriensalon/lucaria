#include <lucaria/public/component_model.hpp>

namespace lucaria {

component_model_blockout& component_model_blockout::use_mesh(const handle_mesh mesh)
{
    _mesh = mesh;
    return *this;
}

component_model_unlit& component_model_unlit::use_mesh(const handle_mesh mesh)
{
    _mesh = mesh;
    return *this;
}

component_model_unlit& component_model_unlit::use_color(const handle_texture color)
{
    _color = color;
    return *this;
}

}
