#include <lucaria/forward/context_dynamics.hpp>
#include <lucaria/core/systems_dynamics.hpp>

namespace lucaria {

// std::optional<collision> context_dynamics::raycast(const float32x3& from, const float32x3& to)
// {
// 	return _system->raycast()
// }

void context_dynamics::set_world_gravity(const float32 newtons)
{
    _system->set_world_gravity(newtons);
}

}