#include <lucaria/core/error.hpp>
#include <lucaria/core/openal.hpp>
#include <lucaria/ecs/component/speaker.hpp>

namespace lucaria {
namespace ecs {

    speaker_component::speaker_component()
    {
        alGenSources(1, &_handle);
        if (!_handle) {
            LUCARIA_RUNTIME_ERROR("Failed to generate OpenAL source")
        }
        _is_owning = true;
    }

    speaker_component::speaker_component(speaker_component&& other)
    {
        *this = std::move(other);
    }

    speaker_component& speaker_component::operator=(speaker_component&& other)
    {
        _is_owning = true;
        _handle = other._handle;
        other._is_owning = false;
        return *this;
    }

    speaker_component::~speaker_component()
    {
        if (_is_owning) {
            alDeleteSources(1, &_handle);
        }
    }

    speaker_component& speaker_component::use_sound(sound& from)
    {
        _sound.emplace(from);
        alSourcei(_handle, AL_BUFFER, _sound.value().get_handle());
        if (_is_playing) {
            alSourcePlay(_sound.value().get_handle());
        }
        return *this;
    }

    speaker_component& speaker_component::use_sound(fetched<sound>& from)
    {
        _sound.emplace(from, [this]() {
            alSourcei(_handle, AL_BUFFER, _sound.value().get_handle());
            if (_is_playing) {
                alSourcePlay(_sound.value().get_handle());
            }
        });
        return *this;
    }

    speaker_component& speaker_component::set_volume(const glm::float32 volume)
    {
        // todo
        return *this;
    }

    speaker_component& speaker_component::set_play(const bool play)
    {
        if (_sound.has_value() && (_is_playing != play)) {
            if (play) {
                alSourcePlay(_sound.value().get_handle());
            } else {
                alSourcePause(_sound.value().get_handle());
            }
            _is_playing = play;
        }
        return *this;
    }

    speaker_component& speaker_component::set_loop(const bool loop)
    {
        // todo
        return *this;
    }

}
}
