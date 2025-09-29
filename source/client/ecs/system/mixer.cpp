#include <lucaria/core/openal.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/ecs/component/speaker.hpp>
#include <lucaria/ecs/system/mixer.hpp>

namespace lucaria {
namespace {

    // void setListenerVelocity(float x, float y, float z)
    // {
    //     alListener3f(AL_VELOCITY, x, y, z);
    //     ALenum error = alGetError();
    //     if (error != AL_NO_ERROR) {
    //         std::cerr << "Error setting listener velocity: " << error << std::endl;
    //     }
    // }

    std::optional<std::reference_wrapper<ecs::transform_component>> listener_transform = std::nullopt;

}

namespace ecs {
    namespace mixer {

        void use_listener_transform(transform_component& transform)
        {
            on_audio_locked([&]() {
                listener_transform = transform;
            });
        }
    }
}

namespace detail {

    void mixer_system::apply_speaker_transforms()
    {
        each_scene([&](entt::registry& scene) {
            scene.view<ecs::speaker_component>().each([](ecs::speaker_component& _speaker) {
                if (_speaker._sound.has_value() && _speaker._want_playing != _speaker._is_playing) {
                    if (_speaker._want_playing) {
                        alSourcePlay(_speaker._sound.value().get_handle());
                    } else {
                        alSourcePause(_speaker._sound.value().get_handle());
                    }
                    _speaker._is_playing = _speaker._want_playing;
                }
            });
            scene.view<ecs::speaker_component, ecs::transform_component>().each([](ecs::speaker_component& _speaker, ecs::transform_component& _transform) {
                if (_speaker._sound.has_value()) {
                    glm::vec3 _position = _transform.get_position();
                    glm::vec3 _forward = _transform.get_forward();
                    alSource3f(_speaker._sound.value().get_handle(), AL_POSITION, _position.x, _position.y, _position.z);
                    alSource3f(_speaker._sound.value().get_handle(), AL_DIRECTION, _forward.x, _forward.y, _forward.z);
                    alSourcei(_speaker._sound.value().get_handle(), AL_SOURCE_RELATIVE, AL_FALSE);
                }
            });
        });
    }

    void mixer_system::apply_listener_transform()
    {
        if (listener_transform.has_value()) {
            const glm::mat4& _transform = listener_transform.value().get()._transform;
            const glm::vec3 _position = glm::vec3(_transform[3]);
            const glm::mat3 R(_transform);
            glm::vec3 at = -glm::normalize(glm::vec3(R[2])); // -Z forward
            glm::vec3 up = glm::normalize(glm::vec3(R[1])); // +Y up

            // Re-orthogonalize up in case of scale/shear
            up = glm::normalize(up - glm::dot(up, at) * at);

            float _orientation[6] = { at.x, at.y, at.z, up.x, up.y, up.z };

            alListener3f(AL_POSITION, _position.x, _position.y, _position.z);
            alListenerfv(AL_ORIENTATION, _orientation);
        }
    }

}
}
