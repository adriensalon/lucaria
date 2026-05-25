
#include <lucaria/public/component_animator.hpp>
#include <lucaria/public/component_interface.hpp>
#include <lucaria/public/component_model.hpp>
#include <lucaria/public/component_rigidbody.hpp>
#include <lucaria/public/component_speaker.hpp>
#include <lucaria/public/component_transform.hpp>
#include <lucaria/public/context_scene.hpp>

namespace lucaria {

handle_entity context_scene::create_entity()
{
    handle_entity _entity;
    _entity._entity = _manager->current_scene->components.create();
    _entity._registry = &_manager->current_scene->components;
    return _entity;
}

void context_scene::mark_erase_entity(const handle_entity entity)
{
    _manager->current_scene->entities_marked_erase.emplace_back(entity._entity);
}

void context_scene::mark_erase_self()
{
    _manager->current_scene->is_marked_erase = true;
}

component_animator& context_scene::create_animator(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_animator>(entity._entity);
}

component_interface_screen& context_scene::create_screen_interface(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_interface_screen>(entity._entity);
}

component_interface_spatial& context_scene::create_spatial_interface(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_interface_spatial>(entity._entity);
}

component_model_blockout& context_scene::create_blockout_model(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_model_blockout>(entity._entity);
}

component_model_unlit& context_scene::create_unlit_model(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_model_unlit>(entity._entity);
}

component_rigidbody_passive& context_scene::create_passive_rigidbody(context_dynamics& dynamics, const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_rigidbody_passive>(entity._entity, dynamics);
}

component_rigidbody_kinematic& context_scene::create_kinematic_rigidbody(context_dynamics& dynamics, const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_rigidbody_kinematic>(entity._entity, dynamics);
}

component_rigidbody_dynamic& context_scene::create_dynamic_rigidbody(context_dynamics& dynamics, const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_rigidbody_dynamic>(entity._entity, dynamics);
}

component_speaker_spatial& context_scene::create_speaker(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_speaker_spatial>(entity._entity);
}

component_transform& context_scene::create_transform(const handle_entity entity)
{
    return _manager->current_scene->components.emplace<component_transform>(entity._entity);
}

}