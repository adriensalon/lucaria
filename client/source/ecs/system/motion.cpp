
#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/world.hpp>

void motion_system::update()
{
    world_system::for_each([](entt::registry& _registry) {
        _registry.view<model_component, animator_component>().each([](model_component& _model, animator_component& _animator) {
            if (_model._mesh.has_value() && _animator._skeleton.has_value()) {
                
                

            }
        });
    });
}