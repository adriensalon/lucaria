#include <glm/gtc/constants.hpp>

#include <ecs/component/rigidbody.hpp>
#include <ecs/system/dynamics.hpp>

rigidbody_component::rigidbody_component(rigidbody_component&& other)
{
    *this = std::move(other);
}

rigidbody_component& rigidbody_component::operator=(rigidbody_component&& other)
{
    _shape = other._shape;
    // _state = other._state;
    // _rigidbody = other._rigidbody;
    _ghost = other._ghost;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

rigidbody_component::~rigidbody_component()
{
    if (_is_instanced) {
        // delete _rigidbody->getMotionState();
        // delete _rigidbody;
        delete _ghost;
        delete _shape;
    }
}

rigidbody_component& rigidbody_component::box(const glm::vec3& half_extents)
{
    _shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.z));
    // _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    // _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape, btVector3(0, 0, 0)));
    // _rigidbody->setCollisionFlags(_rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    // _rigidbody->setActivationState(DISABLE_DEACTIVATION);
    _ghost = new btPairCachingGhostObject();
    _ghost->setCollisionShape(_shape);
    // dynamics_system::get_dynamics_world()->addRigidBody(_rigidbody);
    dynamics_system::get_dynamics_world()->addCollisionObject(_ghost, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter);

    _is_instanced = true;
    return *this;
}

rigidbody_component& rigidbody_component::capsule(const glm::float32 radius, const glm::float32 height)
{
    _shape = new btCapsuleShape(radius, height);
    // _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    // _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape, btVector3(0, 0, 0)));
    // _rigidbody->setCollisionFlags(_rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    // _rigidbody->setActivationState(DISABLE_DEACTIVATION);
    _ghost = new btPairCachingGhostObject();
    _ghost->setCollisionShape(_shape);
    _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    _ghost->setUserIndex(33);
    // dynamics_system::get_dynamics_world()->addRigidBody(_rigidbody);
    dynamics_system::get_dynamics_world()->addCollisionObject(_ghost, btBroadphaseProxy::SensorTrigger,btBroadphaseProxy::StaticFilter);


    _is_instanced = true;
    return *this;
}
