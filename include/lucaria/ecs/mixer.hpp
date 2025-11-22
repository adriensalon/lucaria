#pragma once

#include <lucaria/ecs/transform.hpp>

namespace lucaria {

/// @brief Uses a transform component as the audio listener
/// @param transform the transform component to use
void use_listener_transform(transform_component& transform);

}
