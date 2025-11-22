#include <lucaria/core/error.hpp>
#include <lucaria/core/sound.hpp>

#include <AL/al.h>
#include <AL/alc.h>

namespace lucaria {

extern void _fetch_bytes(const std::filesystem::path& file_path, const std::function<void(const std::vector<char>&)>& callback, bool persist);

sound_track::sound_track(sound_track&& other)
{
    *this = std::move(other);
}

sound_track& sound_track::operator=(sound_track&& other)
{
    if (_is_owning) {
        LUCARIA_RUNTIME_ERROR("Object already owning resources")
    }
    _is_owning = true;
    _handle = other._handle;
    other._is_owning = false;
    return *this;
}

sound_track::~sound_track()
{
    if (_is_owning) {
        alDeleteBuffers(1, &_handle);
    }
}

sound_track::sound_track(const audio& from)
{
    _handle = 0;
    alGenBuffers(1, &_handle);
#if LUCARIA_CONFIG_DEBUG
    if (_handle == 0) {
        LUCARIA_RUNTIME_ERROR("Failed to generate OpenAL buffer")
    }
#endif
    alBufferData(_handle, alGetEnumValue("AL_FORMAT_MONO_FLOAT32"), from.data.samples.data(), static_cast<ALsizei>(from.data.samples.size() * sizeof(glm::float32)), from.data.sample_rate);
#if LUCARIA_CONFIG_DEBUG
    std::cout << "Created sound buffer of size " << from.data.samples.size() << " with id " << _handle << std::endl;
#endif
    _is_owning = true;
}

glm::uint sound_track::get_handle() const
{
    return _handle;
}

fetched<sound_track> fetch_sound(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<audio>> _audio_promise = std::make_shared<std::promise<audio>>();
    _fetch_bytes(data_path, [_audio_promise](const std::vector<char>& _data_bytes) {
        audio _audio(_data_bytes);
        _audio_promise->set_value(std::move(_audio)); }, true);

    // create sound on main thread
    return fetched<sound_track>(_audio_promise->get_future(), [](const audio& _from) {
        return sound_track(_from);
    });
}

}
