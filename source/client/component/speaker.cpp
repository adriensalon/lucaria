#include <lucaria/component/speaker.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/openal.hpp>

namespace lucaria {

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
    _is_playing = false;
    alSourceStop(_handle);
    alSourcei(_handle, AL_BUFFER, _sound.value().get_handle());
    return *this;
}

speaker_component& speaker_component::use_sound(fetched<sound>& from)
{
    _sound.emplace(from, [this]() {
        _is_playing = false;
        alSourceStop(_handle);
        alSourcei(_handle, AL_BUFFER, _sound.value().get_handle());
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
    _want_playing = play;
    return *this;
}

speaker_component& speaker_component::set_loop(const bool loop)
{
    _want_looping = loop;
    return *this;
}

}
