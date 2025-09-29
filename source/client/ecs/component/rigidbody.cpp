#include <btBulletDynamicsCommon.h>
#include <glm/gtc/constants.hpp>

#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/system/dynamics.hpp>

namespace lucaria {
namespace detail {

    extern btDiscreteDynamicsWorld* dynamics_world;

}

namespace ecs {

    // kinematic

    rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::use_shape(shape& from)
    {
        _shape.emplace(from);

        _ghost = std::make_unique<btPairCachingGhostObject>();
        _ghost->setCollisionShape(_shape.value().get_handle());
        _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        detail::dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
        _is_added = true;

        return *this;
    }

    rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::use_shape(fetched<shape>& from)
    {
        _shape.emplace(from, [this]() {
            _ghost = std::make_unique<btPairCachingGhostObject>();
            _ghost->setCollisionShape(_shape.value().get_handle());
            _ghost->setCollisionFlags(_ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
            detail::dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
            _is_added = true;
        });

        return *this;
    }

    rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::set_collide_grounds(const bool enabled)
    {
        _is_snap_ground = enabled;
        return *this;
    }

    rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::set_collide_walls(const bool enabled)
    {
        if (enabled) {
            _mask = _mask | detail::bulletgroupID_collider_wall;
        } else if (contains_layer(_mask, detail::bulletgroupID_collider_wall)) {
            _mask = remove_layer(_mask, detail::bulletgroupID_collider_wall);
        }
        if (_is_added) {
            detail::dynamics_world->removeCollisionObject(_ghost.get());
            detail::dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
        }
        return *this;
    }

    rigidbody_component<rigidbody_kind::kinematic>& rigidbody_component<rigidbody_kind::kinematic>::set_collide_layer(const kinematic_layer layer, const bool enabled)
    {
        const std::int16_t _layer = static_cast<std::int16_t>(layer);
        if (enabled) {
            _mask = _mask | _layer;
        } else if (contains_layer(_mask, _layer)) {
            _mask = remove_layer(_mask, _layer);
        }
        if (_is_added) {
            detail::dynamics_world->removeCollisionObject(_ghost.get());
            detail::dynamics_world->addCollisionObject(_ghost.get(), _group, _mask);
        }
        return *this;
    }

    const std::optional<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_ground_collision() const
    {
        return _ground_collision;
    }

    const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_wall_collisions() const
    {
        return _wall_collisions;
    }

    const std::vector<kinematic_collision>& rigidbody_component<rigidbody_kind::kinematic>::get_layer_collisions(const kinematic_layer layer) const
    {
        return _layer_collisions.at(layer);
    }

    // dynamic

    rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::use_shape(shape& from)
    {
        _shape.emplace(from);
        _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
        _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(_mass, _state.get(), _shape.value().get_handle(), btVector3(0, 0, 0)));
        detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        _is_added = true;

        return *this;
    }

    rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::use_shape(fetched<shape>& from)
    {
        _shape.emplace(from, [this]() {
            _state = std::make_unique<btDefaultMotionState>(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
            _rigidbody = std::make_unique<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(_mass, _state.get(), _shape.value().get_handle(), btVector3(0, 0, 0)));
            detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
            _is_added = true;
        });

        return *this;
    }

    rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_mass(const glm::float32 kilograms)
    {
        _mass = kilograms;
        if (_is_added) {
            _rigidbody->setMassProps(_mass, btVector3(0.f, 0.f, 0.f));
        }

        return *this;
    }

    rigidbody_component<rigidbody_kind::dynamic>& rigidbody_component<rigidbody_kind::dynamic>::set_collide_dynamics(const bool enabled)
    {
        if (enabled) {
            _mask = _mask | detail::bulletgroupID_dynamic_rigidbody;
        } else if (contains_layer(_mask, detail::bulletgroupID_dynamic_rigidbody)) {
            _mask = remove_layer(_mask, detail::bulletgroupID_dynamic_rigidbody);
        }
        if (_is_added) {
            detail::dynamics_world->removeRigidBody(_rigidbody.get());
            detail::dynamics_world->addRigidBody(_rigidbody.get(), _group, _mask);
        }

        return *this;
    }

}
}
