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
    _state = other._state;
    _rigidbody = other._rigidbody;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

rigidbody_component::~rigidbody_component()
{
    if (_is_instanced) {
        delete _rigidbody->getMotionState();
        delete _rigidbody;
        delete _shape;
    }
}

rigidbody_component& rigidbody_component::box(const glm::vec3& half_extents)
{
    _shape = new btBoxShape(btVector3(half_extents.x, half_extents.y, half_extents.z));
    _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape, btVector3(0, 0, 0)));
    _rigidbody->setCollisionFlags(_rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    dynamics_system::get_dynamics_world()->addRigidBody(_rigidbody);
#if LUCARIA_GUIZMO
    std::vector<glm::vec3> _positions = {
        glm::vec3(-half_extents.x, -half_extents.y, -half_extents.z),
        glm::vec3(half_extents.x, -half_extents.y, -half_extents.z),
        glm::vec3(half_extents.x, half_extents.y, -half_extents.z),
        glm::vec3(-half_extents.x, half_extents.y, -half_extents.z),
        glm::vec3(-half_extents.x, -half_extents.y, half_extents.z),
        glm::vec3(half_extents.x, -half_extents.y, half_extents.z),
        glm::vec3(half_extents.x, half_extents.y, half_extents.z),
        glm::vec3(-half_extents.x, half_extents.y, half_extents.z)
    };
    std::vector<glm::uvec2> _indices = {
        glm::uvec2(0, 1), glm::uvec2(1, 2), glm::uvec2(2, 3), glm::uvec2(3, 0),
        glm::uvec2(4, 5), glm::uvec2(5, 6), glm::uvec2(6, 7), glm::uvec2(7, 4),
        glm::uvec2(0, 4), glm::uvec2(1, 5), glm::uvec2(2, 6), glm::uvec2(3, 7)
    };
    _guizmo = std::make_unique<guizmo_mesh_ref>(_positions, _indices);
#endif
    _is_instanced = true;
    return *this;
}

rigidbody_component& rigidbody_component::capsule(const glm::float32 radius, const glm::float32 height)
{
    _shape = new btCapsuleShape(radius, height);
    _state = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    _rigidbody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(0, _state, _shape, btVector3(0, 0, 0)));
    _rigidbody->setCollisionFlags(_rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);

    // TODO REGISTER TO APPROPRIATE DYNAMICS WORLD FROM THE SYSTEM
    // dynamicsWorld->addRigidBody(_rigidbody);
#if LUCARIA_GUIZMO
    constexpr glm::uint _segments = 10;
    std::vector<glm::vec3> _positions;
    std::vector<glm::uvec2> _indices;
    glm::float32 _half_height = height * 0.5f;
    for (glm::uint _j = 0; _j <= _segments / 2; ++_j) {
        glm::float32 _phi = glm::half_pi<glm::float32>() * glm::float32(_j) / glm::float32(_segments / 2);
        for (glm::uint _i = 0; _i <= _segments; ++_i) {
            glm::float32 _theta = glm::two_pi<glm::float32>() * glm::float32(_i) / glm::float32(_segments);
            glm::float32 _x = radius * cosf(_theta) * sinf(_phi);
            glm::float32 _y = radius * cosf(_phi);
            glm::float32 _z = radius * sinf(_theta) * sinf(_phi);
            _positions.push_back(glm::vec3(_x, _half_height + _y, _z));
            if (_i > 0 && _j > 0) {
                _indices.push_back(glm::uvec2(_positions.size() - 2, _positions.size() - 1));
            }
            if (_j > 0) {
                _indices.push_back(glm::uvec2(_positions.size() - 1, _positions.size() - 1 - (_segments + 1)));
            }
        }
    }
    glm::uint offset = _positions.size();
    for (glm::uint _j = 0; _j <= _segments / 2; ++_j) {
        glm::float32 _phi = glm::half_pi<glm::float32>() * glm::float32(_j) / glm::float32(_segments / 2);
        for (glm::uint _i = 0; _i <= _segments; ++_i) {
            glm::float32 _theta = glm::two_pi<glm::float32>() * glm::float32(_i) / glm::float32(_segments);
            glm::float32 _x = radius * cosf(_theta) * sinf(_phi);
            glm::float32 _y = radius * cosf(_phi);
            glm::float32 _z = radius * sinf(_theta) * sinf(_phi);
            _positions.push_back(glm::vec3(_x, -_half_height - _y, _z));
            if (_i > 0 && _j > 0) {
                _indices.push_back(glm::uvec2(_positions.size() - 2, _positions.size() - 1));
            }
            if (_j > 0) {
                _indices.push_back(glm::uvec2(_positions.size() - 1, _positions.size() - 1 - (_segments + 1)));
            }
        }
    }
    _guizmo = std::make_unique<guizmo_mesh_ref>(_positions, _indices);
#endif
    _is_instanced = true;
    return *this;
}
