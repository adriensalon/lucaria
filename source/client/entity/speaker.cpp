#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/error.hpp>
#include <lucaria/entity/speaker.hpp>

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
	_is_owning = false;
    *this = std::move(other);
}

speaker_component& speaker_component::operator=(speaker_component&& other)
{
    _is_owning = true;
    _handle = other._handle;
    _sound = std::move(other._sound);
    _is_playing = other._is_playing;
    _want_playing = other._want_playing;
    _is_looping = other._is_looping;
    _want_looping = other._want_looping;
	// TODO CLEANER
    other._is_owning = false;
    return *this;
}

speaker_component::~speaker_component()
{
    if (_is_owning) {
        alDeleteSources(1, &_handle);
    }
}

speaker_component& speaker_component::use_sound(const sound_track_object sound_track)
{
    _sound = sound_track;
    _sound._resource->on_ready([this]() {
        _is_playing = false;
        alSourceStop(_handle);
        alSourcei(_handle, AL_BUFFER, _sound._resource->get().id);
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

std::optional<glm::uint> speaker_component::get_sample_rate() const
{
    if (!_sound) {
        return std::nullopt;
    }
    return _sound._resource->get().sample_rate;
}

std::optional<glm::uint> speaker_component::get_count() const
{
    if (!_sound) {
        return std::nullopt;
    }
    return _sound._resource->get().samples_count;
}
}

