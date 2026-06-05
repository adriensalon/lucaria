#include <entt/entt.hpp>

#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>
#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {

bool handle_entity::has_value() const
{
    return _entity != entt::null;
}



}