#pragma once

#include <glm/glm.hpp>

struct rigidbody_component {



    // collision volume ici aussi

    rigidbody_component& mass(const glm::float32 value);

private:

    friend struct dynamics_system;
};