#include <glm/gtc/constants.hpp>

#include <ecs/component/rigidbody.hpp>

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
    
    // TODO REGISTER TO APPROPRIATE DYNAMICS WORLD FROM THE SYSTEM
    // dynamicsWorld->addRigidBody(_rigidbody);
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
    const int segments = 16; // Number of segments for the wireframe sphere/capsule representation
    std::vector<glm::vec3> _positions;
    std::vector<glm::uvec2> _indices;

    float halfHeight = height * 0.5f;

    // Generate vertices for the cylinder sides
    for (int i = 0; i <= segments; ++i) {
        float theta = glm::two_pi<float>() * float(i) / float(segments);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        _positions.push_back(glm::vec3(x, halfHeight, z));
        _positions.push_back(glm::vec3(x, -halfHeight, z));
        if (i > 0) {
            _indices.push_back(glm::uvec2(2 * i - 2, 2 * i));
            _indices.push_back(glm::uvec2(2 * i - 1, 2 * i + 1));
        }
    }
    _indices.push_back(glm::uvec2(0, 2 * segments));
    _indices.push_back(glm::uvec2(1, 2 * segments + 1));

    // Generate vertices for the top hemisphere
    int offset = _positions.size();
    for (int j = 1; j < segments / 2; ++j) {
        float phi = glm::half_pi<float>() * float(j) / float(segments / 2);
        for (int i = 0; i <= segments; ++i) {
            float theta = glm::two_pi<float>() * float(i) / float(segments);
            float x = radius * cosf(theta) * sinf(phi);
            float y = radius * cosf(phi);
            float z = radius * sinf(theta) * sinf(phi);
            _positions.push_back(glm::vec3(x, halfHeight + y, z));
            if (i > 0) {
                _indices.push_back(glm::uvec2(offset + (i - 1) + (j - 1) * (segments + 1), offset + i + (j - 1) * (segments + 1)));
            }
            if (j > 1) {
                _indices.push_back(glm::uvec2(offset + i + (j - 2) * (segments + 1), offset + i + (j - 1) * (segments + 1)));
            }
        }
    }

    // Generate vertices for the bottom hemisphere
    offset = _positions.size();
    for (int j = 1; j < segments / 2; ++j) {
        float phi = glm::half_pi<float>() * float(j) / float(segments / 2);
        for (int i = 0; i <= segments; ++i) {
            float theta = glm::two_pi<float>() * float(i) / float(segments);
            float x = radius * cosf(theta) * sinf(phi);
            float y = radius * cosf(phi);
            float z = radius * sinf(theta) * sinf(phi);
            _positions.push_back(glm::vec3(x, -halfHeight - y, z));
            if (i > 0) {
                _indices.push_back(glm::uvec2(offset + (i - 1) + (j - 1) * (segments + 1), offset + i + (j - 1) * (segments + 1)));
            }
            if (j > 1) {
                _indices.push_back(glm::uvec2(offset + i + (j - 2) * (segments + 1), offset + i + (j - 1) * (segments + 1)));
            }
        }
    }

    _guizmo = std::make_unique<guizmo_mesh_ref>(_positions, _indices);
#endif
    _is_instanced = true;
    return *this;
}
