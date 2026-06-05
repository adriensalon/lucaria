#include <lucaria/forward/context_mixer.hpp>

namespace lucaria {

void context_mixer::use_listener_transform(component_transform& transform)
{
	_system->listener_transform = &transform;
}

void context_mixer::set_listener_volume(const float32 volume)
{
	// TODO
}

}