#pragma once

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

struct rigidbody_component {



    // collision volume ici aussi

    rigidbody_component& mass(const glm::float32 value);

private:
    btRigidBody* _box_rigidbody = nullptr;
    friend struct dynamics_system;
};