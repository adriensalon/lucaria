
#include <ecs/system/mixer.hpp>

#include <game/gameplay/runner.hpp>

#include <core/font.hpp>
#include <core/window.hpp>

constexpr glm::uint animationID_player_lol1 = 44;
constexpr glm::uint animationID_player_lol2 = 45;

std::unique_ptr<runner_actor> faith = nullptr;

void level_persistent_player(entt::registry& registry)
{
    faith = std::make_unique<runner_actor>(registry);

    faith->add_script([](runner_actor& runner) {
        if (get_keys()["i"]) {
            runner.get_transform().position_relative({ 0.03f, 0.f, -0.005f });
        }
        if (get_keys()["k"]) {
            runner.get_animator().get_controller(444).play();
        } else {
            runner.get_animator().get_controller(444).pause();
        }
    });

    mixer_system::use_listener_transform(faith->get_transform());
}