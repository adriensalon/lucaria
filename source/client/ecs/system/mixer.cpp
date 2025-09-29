#include <lucaria/core/openal.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/speaker.hpp>
#include <lucaria/ecs/system/mixer.hpp>

namespace lucaria {
namespace {

    std::optional<std::reference_wrapper<ecs::transform_component>> listener_transform = std::nullopt;

}

namespace ecs {
    namespace mixer {

        void use_listener_transform(transform_component& transform)
        {
            listener_transform = transform;
        }
    }
}

namespace detail {

    void mixer_system::apply_speaker_transforms()
    {
        if (listener_transform.has_value()) {
            detail::each_scene([&](entt::registry& scene) {
                scene.view<ecs::speaker_component>().each([](ecs::speaker_component& _speaker) {
                    if (_speaker._sound.has_value() && _speaker._want_playing != _speaker._is_playing) {

                        if (_speaker._want_playing) {
                            alSourcePlay(_speaker._handle);

                        } else {
                            alSourcePause(_speaker._handle);
                        }

                        _speaker._is_playing = _speaker._want_playing;
                    }
                });
                
                scene.view<ecs::speaker_component, ecs::transform_component>().each([](ecs::speaker_component& _speaker, ecs::transform_component& _transform) {
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

    void mixer_system::apply_listener_transform()
    {
        if (listener_transform.has_value()) {
            const ecs::transform_component& _transform = listener_transform->get();

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

}
}
