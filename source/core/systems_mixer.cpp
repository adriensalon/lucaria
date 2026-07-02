#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/systems_mixer.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>

namespace lucaria {
namespace detail {


    void system_mixer::use_listener_transform(handle_entity entity)
    {
        listener_entity = entity;
        listener_transform = nullptr;
    }

    void system_mixer::resolve_runtime_references(manager_scenes& scenes)
    {
        listener_transform = nullptr;
        if (!listener_entity || !listener_entity->has_value()) {
            return;
        }
        listener_transform = scenes.registry.try_get<component_transform>(listener_entity->_entity);
    }

    void system_mixer::update_apply_speaker_transforms(manager_scenes& scenes)
    {
        resolve_runtime_references(scenes);
        if (listener_transform) {
            scenes.each_view<component_speaker_spatial>([](component_speaker_spatial& _speaker) {
                if (_speaker._sound && _speaker._want_playing != _speaker._is_playing) {

                    if (_speaker._want_playing) {
                        alSourcePlay(_speaker._handle);

                    } else {
                        alSourceStop(_speaker._handle);
                    }

                    _speaker._is_playing = _speaker._want_playing;
                }
            });

            scenes.each_view<component_speaker_spatial, component_transform>([](component_speaker_spatial& _speaker, component_transform& _transform) {
                if (_speaker._sound) {
                    const ALuint _handle = _speaker._handle;
                    const float32x3 _position = _transform.get_position();
                    const float32x3 _forward = _transform.get_forward();

                    alSourcei(_handle, AL_SOURCE_RELATIVE, AL_FALSE);
                    alSource3f(_handle, AL_POSITION, _position.x, _position.y, _position.z);

                    // AL_DIRECTION only matters if we use a cone otherwise we can skip it
                    alSource3f(_handle, AL_DIRECTION, _forward.x, _forward.y, _forward.z);
                }
            });
        }
    }

    void system_mixer::update_apply_listener_transform(manager_scenes& scenes)
    {
        resolve_runtime_references(scenes);
        if (listener_transform) {
            const float32x3 _position = listener_transform->get_position();
            const float32x3 _forward = listener_transform->get_forward();
            float32x3 _up = listener_transform->get_up();

            // reorthogonalize (protects against scaling)
            _up = glm::normalize(_up - glm::dot(_up, _forward) * _forward);

            const float _orientation[6] = { _forward.x, _forward.y, _forward.z, _up.x, _up.y, _up.z };
            alListener3f(AL_POSITION, _position.x, _position.y, _position.z);
            alListenerfv(AL_ORIENTATION, _orientation);
        }
    }

}
}
