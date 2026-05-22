#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/database.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/sound_track.hpp>

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

    sound_track_implementation::~sound_track_implementation()
    {
        if (ownership.owns()) {
            alDeleteBuffers(1, &id);
        }
    }

    sound_track_implementation::sound_track_implementation(const audio_implementation& from)
        : origin(from.origin == audio_origin::path ? sound_track_origin::path : sound_track_origin::data)
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
        ownership.emplace();
    }

    sound_track_recipe make_recipe(const implementation_container<sound_track_implementation>& container)
    {
        const sound_track_implementation& _sound_track = container.fetched.value();

        if (_sound_track.origin == sound_track_origin::path) {
            return sound_track_path_recipe { container.origin_path.value() };
        }

        // else if (_sound_track.origin == sound_track_origin::data) {
        //     return sound_track_data_recipe { audio_data { _sound_track.sample_rate, std::vector<float32>(_sound_track.samples_count) } };
        // }
        // BROKEN

        else {
            LUCARIA_RUNTIME_ERROR("Implementation error");
            return {};
        }
    }
}

sound_track_object sound_track_object::fetch(const std::filesystem::path& path)
{
	sound_track_object _sound_track = {};
    _sound_track._resource = detail::engine_resources().sound_tracks.get_or_create_by_path(path, [&] {
        return detail::_fetch_sound_track_async(path);
    });
    _sound_track._manager = &detail::engine_resources().sound_tracks;
    _sound_track._refcount.emplace();
    return _sound_track;
}

bool sound_track_object::has_value() const
{
    return _resource && _resource->fetched.has_value();
}

sound_track_object::operator bool() const
{
    return has_value();
}

}
