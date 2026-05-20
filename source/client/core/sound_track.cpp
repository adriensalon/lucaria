#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/sound_track.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace detail {

    namespace {

        async_container<sound_track_implementation> _fetch_sound_track_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<audio_implementation>> _audio_promise = std::make_shared<std::promise<audio_implementation>>();
        	fetch_bytes(path, [_audio_promise](const std::vector<char>& _bytes) {
        audio_implementation _audio(_bytes);
        _audio_promise->set_value(std::move(_audio)); }, true);

            // create sound on main thread
            return async_container<sound_track_implementation>(_audio_promise->get_future(), [](const audio_implementation& _audio) {
                return sound_track_implementation(_audio);
            });
        }

    }

    sound_track_implementation::sound_track_implementation(sound_track_implementation&& other)
    {
        *this = std::move(other);
    }

    sound_track_implementation& sound_track_implementation::operator=(sound_track_implementation&& other)
    {
        if (is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }
        is_owning = true;
        id = other.id;
        sample_rate = other.sample_rate;
        samples_count = other.samples_count;
        other.is_owning = false;
        return *this;
    }

    sound_track_implementation::~sound_track_implementation()
    {
        if (is_owning) {
            alDeleteBuffers(1, &id);
        }
    }

    sound_track_implementation::sound_track_implementation(const audio_implementation& from)
    {
        id = 0;
        sample_rate = from.data.sample_rate;
        samples_count = static_cast<glm::uint>(from.data.samples.size());
        alGenBuffers(1, &id);
#if defined(LUCARIA_DEBUG)
        if (id == 0) {
            LUCARIA_RUNTIME_ERROR("Failed to generate OpenAL buffer")
        }
#endif
        alBufferData(id, alGetEnumValue("AL_FORMAT_MONO_FLOAT32"), from.data.samples.data(), static_cast<ALsizei>(from.data.samples.size() * sizeof(glm::float32)), from.data.sample_rate);
// #if defined(LUCARIA_DEBUG)
//         std::cout << "Created sound buffer of size " << from.data.samples.size() << " with id " << id << std::endl;
// #endif
        is_owning = true;
    }
}

sound_track_object sound_track_object::fetch(const std::filesystem::path& path)
{
    detail::resource_container<detail::sound_track_implementation>* _resource = detail::engine_resources().sound_tracks.get_or_create_by_path(path, [&] {
        return detail::_fetch_sound_track_async(path);
    });

    return sound_track_object { _resource };
}

bool sound_track_object::has_value() const
{
    return _resource && _resource->is_ready();
}

sound_track_object::operator bool() const
{
    return has_value();
}

sound_track_object::sound_track_object(detail::resource_container<detail::sound_track_implementation>* resource)
    : _resource(resource)
{
}

}
