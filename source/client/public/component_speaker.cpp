#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/public/component_speaker.hpp>

namespace lucaria {

component_speaker_spatial::component_speaker_spatial()
{
    alGenSources(1, &_handle);
    if (!_handle) {
        LUCARIA_DEBUG_ERROR("Failed to generate OpenAL source")
    }

    _ownership.emplace();
}

component_speaker_spatial::~component_speaker_spatial()
{
    if (_ownership.owns()) {
        alDeleteSources(1, &_handle);
    }
}

component_speaker_spatial& component_speaker_spatial::use_sound(const handle_sound_track sound_track)
{
    _sound = sound_track;
    _sound._cached->fetched.on_ready([this]() {
        _is_playing = false;
        alSourceStop(_handle);
        alSourcei(_handle, AL_BUFFER, _sound._cached->fetched.value().id);
    });
    return *this;
}

component_speaker_spatial& component_speaker_spatial::set_volume(const glm::float32 volume)
{
    // todo
    return *this;
}

component_speaker_spatial& component_speaker_spatial::set_play(const bool play)
{
    _want_playing = play;
    return *this;
}

component_speaker_spatial& component_speaker_spatial::set_loop(const bool loop)
{
    _want_looping = loop;
    return *this;
}

std::optional<glm::uint> component_speaker_spatial::get_sample_rate() const
{
    if (!_sound) {
        return std::nullopt;
    }
    return _sound._cached->fetched.value().sample_rate;
}

std::optional<glm::uint> component_speaker_spatial::get_count() const
{
    if (!_sound) {
        return std::nullopt;
    }
    return _sound._cached->fetched.value().samples_count;
}
}

