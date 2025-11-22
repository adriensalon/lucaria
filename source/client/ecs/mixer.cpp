#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/run.hpp>
#include <lucaria/ecs/mixer.hpp>
#include <lucaria/ecs/speaker.hpp>

namespace lucaria {

void _system_compute_mixer();

namespace {

    std::optional<std::reference_wrapper<transform_component>> listener_transform = std::nullopt;

}

void use_listener_transform(transform_component& transform)
{
    listener_transform = transform;
}

struct mixer_system {

    static void apply_speaker_transforms()
    {
        if (listener_transform.has_value()) {
            each_scene([&](entt::registry& scene) {
                scene.view<speaker_component>().each([](speaker_component& _speaker) {
                    if (_speaker._sound.has_value() && _speaker._want_playing != _speaker._is_playing) {

                        if (_speaker._want_playing) {
                            alSourcePlay(_speaker._handle);

                        } else {
                            alSourceStop(_speaker._handle);
                        }

                        _speaker._is_playing = _speaker._want_playing;
                    }
                });

                scene.view<speaker_component, transform_component>().each([](speaker_component& _speaker, transform_component& _transform) {
                    if (_speaker._sound.has_value()) {
                        const ALuint _handle = _speaker._handle;
                        const glm::vec3 _position = _transform.get_position();
                        const glm::vec3 _forward = _transform.get_forward();

                        alSourcei(_handle, AL_SOURCE_RELATIVE, AL_FALSE);
                        alSource3f(_handle, AL_POSITION, _position.x, _position.y, _position.z);

                        // AL_DIRECTION only matters if we use a cone otherwise we can skip it
                        alSource3f(_handle, AL_DIRECTION, _forward.x, _forward.y, _forward.z);
                    }
                });
            });
        }
    }

    static void apply_listener_transform()
    {
        if (listener_transform.has_value()) {
            const transform_component& _transform = listener_transform->get();

            const glm::vec3 _position = _transform.get_position();
            const glm::vec3 _forward = _transform.get_forward();
            glm::vec3 _up = _transform.get_up();

            // reorthogonalize (protects against scaling)
            _up = glm::normalize(_up - glm::dot(_up, _forward) * _forward);

            const float _orientation[6] = { _forward.x, _forward.y, _forward.z, _up.x, _up.y, _up.z };
            alListener3f(AL_POSITION, _position.x, _position.y, _position.z);
            alListenerfv(AL_ORIENTATION, _orientation);
        }
    }
};

void _system_compute_mixer()
{
    mixer_system::apply_speaker_transforms();
    mixer_system::apply_listener_transform();
}

}
