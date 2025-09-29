#include <btBulletDynamicsCommon.h>
#include <lucaria/ecs/component/collider.hpp>
#include <lucaria/ecs/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* dynamics_world;

}

namespace ecs {

    collider_component& collider_component::use_shape(shape& from)
    {
        _shape.emplace(from);

        btCollisionShape* _collision_shape = _shape.value().get_handle();

        if (_is_added) {
            detail::dynamics_world->removeRigidBody(_rigidbody.get());
            _rigidbody->setCollisionShape(_collision_shape);

        } else {
            _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1)));
            _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0, _state.get(), _collision_shape));
        }

        detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        _is_added = true;

        return *this;
    }

    collider_component& collider_component::use_shape(fetched<shape>& from)
    {
        _shape.emplace(from, [this]() {
            btCollisionShape* _collision_shape = _shape.value().get_handle();

            if (_is_added) {
                detail::dynamics_world->removeRigidBody(_rigidbody.get());
                _rigidbody->setCollisionShape(_collision_shape);

            } else {
                _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1)));
                _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0, _state.get(), _collision_shape));
            }

            detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
            _is_added = true;
        });

        return *this;
    }

    collider_component& collider_component::set_ground()
    {
        if (_group != detail::bulletgroupID_collider_ground) {
            _group = detail::bulletgroupID_collider_ground;
            _mask = _mask | detail::bulletgroupID_dynamic_rigidbody;
            if (_is_added) {
                detail::dynamics_world->removeRigidBody(_rigidbody.get());
                detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
            }
        }
        return *this;
    }

    collider_component& collider_component::set_wall()
    {
        if (_group != detail::bulletgroupID_collider_wall) {
            _group = detail::bulletgroupID_collider_wall;
            _mask = _mask | detail::bulletgroupID_dynamic_rigidbody;
            if (_is_added) {
                detail::dynamics_world->removeRigidBody(_rigidbody.get());
                detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
            }
        }
        return *this;
    }

    collider_component& collider_component::set_layer(const kinematic_layer layer)
    {
        const std::int16_t _layer = static_cast<std::int16_t>(layer);
        if (_group != _layer) {
            _group = _layer;
            if (contains_layer(_mask, detail::bulletgroupID_dynamic_rigidbody)) {
                _mask = remove_layer(_mask, detail::bulletgroupID_dynamic_rigidbody);
            }
            if (_is_added) {
                detail::dynamics_world->removeRigidBody(_rigidbody.get());
                detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
            }
        }
        return *this;
    }

}
}
