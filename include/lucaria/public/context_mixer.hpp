#pragma once

#include <lucaria/core/system_mixer.hpp>
#include <lucaria/public/component_transform.hpp>
#include <lucaria/public/component_speaker.hpp>

namespace lucaria {

struct context_mixer {

    /// @brief Uses a transform component as the audio listener
    /// @param transform the transform component to use
    void use_listener_transform(component_transform& transform);

	/// @brief 
	/// @param volume 
	void set_listener_volume(const float32 volume);

private:
	detail::system_mixer* _system;
	friend struct access_context;
};

}
