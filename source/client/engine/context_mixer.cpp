#include <lucaria/engine/context_mixer.hpp>

namespace lucaria {

void context_mixer::use_listener_transform(handle_entity entity)
{
	_system->use_listener_transform(entity);
}

void context_mixer::set_listener_volume(const float32 volume)
{
	// TODO
}

}