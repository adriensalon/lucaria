#pragma once

#include <lucaria/entity/transform.hpp>

namespace lucaria {
namespace detail {

	struct mixer_system {
		static void apply_speaker_transforms();
		static void apply_listener_transform();
	};

}

struct mixer_context {

    /// @brief Uses a transform component as the audio listener
    /// @param transform the transform component to use
    void use_listener_transform(transform_component& transform);

	/// @brief 
	/// @param volume 
	void set_listener_volume(const float32 volume);
};

}
