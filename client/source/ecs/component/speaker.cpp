#include <ecs/component/speaker.hpp>

speaker_component& speaker_component::sounds(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<sound_ref>>>& fetched_sounds)
{
    for (const std::pair<const glm::uint, std::shared_future<std::shared_ptr<sound_ref>>>& _pair : fetched_sounds) {
        const glm::uint _name = _pair.first;
        _sounds[_name].emplace(_pair.second, [this, _name] () {
            // todo create source
        });
        _controllers[_name] = sound_controller();
    }
    return *this;
}

sound_controller& speaker_component::get_controller(const glm::uint name)
{
    return _controllers.at(name);
}
