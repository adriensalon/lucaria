#pragma once

#include <lucaria/core/systems_mixer.hpp>
#include <lucaria/engine/component_transform.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {

struct context_mixer {

    /// @brief Uses an entity transform as the audio listener
    /// @param entity the entity containing the transform component
    void use_listener_transform(handle_entity entity);

	/// @brief 
	/// @param volume 
	void set_listener_volume(const float32 volume);

private:
	detail::system_mixer* _system;
	friend struct access_context;
};

}
