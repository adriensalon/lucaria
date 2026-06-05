#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/asset_sound_track.hpp>

namespace lucaria {
namespace detail {

    object_sound_track::~object_sound_track()
    {
        if (ownership.owns()) {
            alDeleteBuffers(1, &id);
        }
    }

    object_sound_track::object_sound_track(const object_audio& from)
        : origin(from.origin == object_audio_origin::path ? object_sound_track_origin::path : object_sound_track_origin::data)
    {
        id = 0;
        sample_rate = from.data.sample_rate;
        samples_count = static_cast<glm::uint>(from.data.samples.size());
        alGenBuffers(1, &id);
#if defined(LUCARIA_DEBUG)
        if (id == 0) {
            LUCARIA_DEBUG_ERROR("Failed to generate OpenAL buffer")
        }
#endif
        alBufferData(id, alGetEnumValue("AL_FORMAT_MONO_FLOAT32"), from.data.samples.data(), static_cast<ALsizei>(from.data.samples.size() * sizeof(glm::float32)), from.data.sample_rate);
        ownership.emplace();
    }

}
}
