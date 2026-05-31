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
    return _entity != entt::null;
}



}