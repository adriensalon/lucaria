#include <lucaria/core/audio.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/ecs/system/mixer.hpp>

namespace detail {

// void setListenerVelocity(float x, float y, float z)
// {
//     alListener3f(AL_VELOCITY, x, y, z);
//     ALenum error = alGetError();
//     if (error != AL_NO_ERROR) {
//         std::cerr << "Error setting listener velocity: " << error << std::endl;
//     }
// }

std::optional<std::reference_wrapper<transform_component>> listener_transform = std::nullopt;

}

void mixer_system::use_listener_transform(transform_component& transform)
{
    on_audio_locked([&] () {
        detail::listener_transform = std::ref(transform);
    });
}

void mixer_system::apply_speaker_transforms()
{
    
}

void mixer_system::apply_listener_transform()
{
    if (detail::listener_transform.has_value()) {
        const glm::mat4& _transform = detail::listener_transform.value().get()._transform;
        const glm::vec3 _position = glm::vec3(_transform[3]);
        glm::mat3 _rotation(_transform);
        const glm::vec3 _forward = glm::normalize(_rotation * glm::vec3(0.0f, 0.0f, -1.0f));
        const glm::vec3 _up = glm::normalize(_rotation * glm::vec3(0.0f, 1.0f, 0.0f));
        glm::float32 _orientation[] = { _forward.x, _forward.y, _forward.z, _up.x, _up.y, _up.z };
        alListener3f(AL_POSITION, _position.x, _position.y, _position.z);        
        alListenerfv(AL_ORIENTATION, _orientation);
    }
}
