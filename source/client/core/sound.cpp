#include <lucaria/core/error.hpp>
#include <lucaria/core/sound.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/openal.hpp>

namespace lucaria {

sound::sound(sound&& other)
{
    *this = std::move(other);
}

sound& sound::operator=(sound&& other)
{
    _is_owning = true;
    _handle = other._handle;
    other._is_owning = false;
    return *this;
}

sound::~sound()
{
    if (_is_owning) {
        alDeleteBuffers(1, &_handle);
    }
}

sound::sound(const audio& from)
{
    _handle = 0;
    alGenBuffers(1, &_handle);
#if LUCARIA_DEBUG
    if (_handle == 0) {
        LUCARIA_RUNTIME_ERROR("Failed to generate OpenAL buffer")
    }
#endif
    alBufferData(_handle, alGetEnumValue("AL_FORMAT_MONO_FLOAT32"), from.data.samples.data(), static_cast<ALsizei>(from.data.samples.size() * sizeof(glm::float32)), from.data.sample_rate);
#if LUCARIA_DEBUG
    std::cout << "Created sound buffer of size " << from.data.samples.size() << " with id " << _handle << std::endl;
#endif
    _is_owning = true;
}

glm::uint sound::get_handle() const
{
    return _handle;
}

fetched<sound> fetch_sound(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<audio>> _audio_promise = std::make_shared<std::promise<audio>>();

    detail::fetch_bytes(data_path, [_audio_promise](const std::vector<char>& _data_bytes) {
        audio _audio(_data_bytes);
        _audio_promise->set_value(std::move(_audio));
    });

    // create sound on main thread
    return fetched<sound>(_audio_promise->get_future(), [](const audio& _from) {
        return sound(_from);
    });
}

}
