#include <lucaria/public/component_animator.hpp>
#include <lucaria/public/component_interface.hpp>
#include <lucaria/public/component_model.hpp>
#include <lucaria/public/component_rigidbody.hpp>
#include <lucaria/public/component_speaker.hpp>
#include <lucaria/public/component_transform.hpp>
#include <lucaria/public/handle_entity.hpp>

namespace lucaria {

bool handle_entity::has_value() const
{
    return (_entity != entt::null) && (_registry != nullptr);
}

component_animator& handle_entity::get_animator() const
{
    return _registry->get<component_animator>(_entity);
}

component_interface_screen& handle_entity::get_screen_interface() const
{
    return _registry->get<component_interface_screen>(_entity);
}

component_interface_spatial& handle_entity::get_spatial_interface() const
{
    return _registry->get<component_interface_spatial>(_entity);
}

component_model_blockout& handle_entity::get_blockout_model() const
{
    return _registry->get<component_model_blockout>(_entity);
}

component_model_unlit& handle_entity::get_unlit_model() const
{
    return _registry->get<component_model_unlit>(_entity);
}

component_rigidbody_passive& handle_entity::get_passive_rigidbody() const
{
    return _registry->get<component_rigidbody_passive>(_entity);
}

component_rigidbody_kinematic& handle_entity::get_kinematic_rigidbody() const
{
    return _registry->get<component_rigidbody_kinematic>(_entity);
}

component_rigidbody_dynamic& handle_entity::get_dynamic_rigidbody() const
{
    return _registry->get<component_rigidbody_dynamic>(_entity);
}

component_speaker_spatial& handle_entity::get_speaker() const
{
    return _registry->get<component_speaker_spatial>(_entity);
}

component_transform& handle_entity::get_transform() const
{
    return _registry->get<component_transform>(_entity);
}

}