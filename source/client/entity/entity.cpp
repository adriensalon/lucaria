#include <lucaria/entity/entity.hpp>
#include <lucaria/entity/animator.hpp>
#include <lucaria/entity/interface.hpp>
#include <lucaria/entity/model.hpp>
#include <lucaria/entity/rigidbody.hpp>
#include <lucaria/entity/speaker.hpp>
#include <lucaria/entity/transform.hpp>

namespace lucaria {

bool scene_entity::has_value() const
{
    return (_entity != entt::null) && (_registry != nullptr);
}

animator_component& scene_entity::get_animator() const
{
	return _registry->get<animator_component>(_entity);
}

screen_interface_component& scene_entity::get_screen_interface() const
{
	return _registry->get<screen_interface_component>(_entity);
}

spatial_interface_component& scene_entity::get_spatial_interface() const
{
	return _registry->get<spatial_interface_component>(_entity);
}

blockout_model_component& scene_entity::get_blockout_model() const
{
	return _registry->get<blockout_model_component>(_entity);
}

unlit_model_component& scene_entity::get_unlit_model() const
{
	return _registry->get<unlit_model_component>(_entity);
}

passive_rigidbody_component& scene_entity::get_passive_rigidbody() const
{
	return _registry->get<passive_rigidbody_component>(_entity);
}

kinematic_rigidbody_component& scene_entity::get_kinematic_rigidbody() const
{
	return _registry->get<kinematic_rigidbody_component>(_entity);
}

dynamic_rigidbody_component& scene_entity::get_dynamic_rigidbody() const
{
	return _registry->get<dynamic_rigidbody_component>(_entity);
}

speaker_component& scene_entity::get_speaker() const
{
	return _registry->get<speaker_component>(_entity);
}

transform_component& scene_entity::get_transform() const
{
	return _registry->get<transform_component>(_entity);
}

}