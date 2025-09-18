#include <lucaria/core/audio.hpp>
#include <lucaria/ecs/component/speaker.hpp>

namespace lucaria {

speaker_component& speaker_component::sounds(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<sound_ref>>>& fetched_sounds)
{
    for (const std::pair<const glm::uint, std::shared_future<std::shared_ptr<sound_ref>>>& _pair : fetched_sounds) {
        const glm::uint _name = _pair.first;
        _sounds[_name].emplace(_pair.second, [this, _name] () {
            alGenSources(1, &(_source_ids[_name]));
#if LUCARIA_DEBUG
            if (!_source_ids.at(_name)) {
                std::cout << "Failed to generate OpenAL source." << std::endl;
                std::terminate();
            }
#endif
            alSourcei(_source_ids[_name], AL_BUFFER, _sounds.at(_name).value().get_id());
            alSourcePlay(_source_ids[_name]); //CONTROVERSIAL
        });
        _controllers[_name] = sound_controller();
    }
    return *this;
}

sound_controller& speaker_component::get_controller(const glm::uint name)
{
    return _controllers.at(name);
}

}
