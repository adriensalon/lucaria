
#include <game/gameplay/runner.hpp>

#include <core/font.hpp>
#include <core/window.hpp>

constexpr glm::uint animationID_player_lol1 = 44;
constexpr glm::uint animationID_player_lol2 = 45;

std::unique_ptr<runner> faith = nullptr;

void level_persistent_player(entt::registry& registry)
{    
    faith = std::make_unique<runner>(registry);

    transform_component* _faith_transform = &faith->get_transform();

    faith->add_script([_faith_transform] (runner_controller_state& state) {
        if (get_keys()["i"]) {
            _faith_transform->position_relative({ 0.03f, 0.f, -0.005f });
        }
    });

    mixer_system::use_listener_transform(*_faith_transform);
}