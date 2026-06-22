#pragma once

#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {
namespace detail {
    struct system_mixer;
}

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
