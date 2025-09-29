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
        if (listener_transform.has_value()) {
            each_scene([&](entt::registry& scene) {
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
                        const ALuint h = _speaker._handle;
                        

                        glm::vec3 pos = _transform.get_position(); // world pos
                        glm::vec3 fwd = _transform.get_forward(); // −Z forward by our convention

                        alSourcei(h, AL_SOURCE_RELATIVE, AL_FALSE);
                        alSource3f(h, AL_POSITION, pos.x, pos.y, pos.z);

                        // `AL_DIRECTION` only matters if you use a cone. Otherwise you can skip it.
                        alSource3f(h, AL_DIRECTION, fwd.x, fwd.y, fwd.z);
                    }
                });
            });
        }
    }

    void mixer_system::apply_listener_transform()
    {
        if (listener_transform.has_value()) {
            const glm::mat4& T = listener_transform->get()._transform;

            glm::vec3 pos = glm::vec3(T[3]);
            glm::vec3 at = glm::normalize(-glm::vec3(T[2])); // −Z forward
            glm::vec3 up = glm::normalize(glm::vec3(T[1])); // +Y up

            // Re-orthogonalize (protects against scaling)
            up = glm::normalize(up - glm::dot(up, at) * at);

            // (Optional) sanity: ensure right-handedness (+X right)
            glm::vec3 right = glm::cross(at, up);
            if (glm::dot(right, glm::vec3(T[0])) < 0.0f) {
                // If this ever trips, your basis is flipped somewhere.
                right = -right; // or log/adjust as you like
            }

            float orientation[6] = { at.x, at.y, at.z, up.x, up.y, up.z };
            alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
            alListenerfv(AL_ORIENTATION, orientation);
        }
    }

}
}
