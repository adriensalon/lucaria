#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/object_sound_track.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        container_async<object_sound_track> _fetch_sound_track_async(manager_assets& objects, const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<object_audio>> _audio_promise = std::make_shared<std::promise<object_audio>>();
            objects.fetch_bytes(path, [_audio_promise](const std::vector<char>& _bytes) {
				object_audio _audio(_bytes);
				_audio_promise->set_value(std::move(_audio)); }, true);

            // create sound on main thread
            return container_async<object_sound_track>(_audio_promise->get_future(), [](const object_audio& _audio) {
                return object_sound_track(_audio);
            });
        }

    }

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
        // #if defined(LUCARIA_DEBUG)
        //         std::cout << "Created sound buffer of size " << from.data.samples.size() << " with id " << id << std::endl;
        // #endif
        ownership.emplace();
    }

    assets_cell<object_sound_track>& fetch(
        manager_assets& objects,
        assets_buffer<object_sound_track>& cached_vector,
        const std::filesystem::path& path)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path] {
            return _fetch_sound_track_async(objects, path);
        });
    }

    recipe_object_sound_track make_recipe(const assets_cell<object_sound_track>& cached)
    {
        const object_sound_track& _sound_track = cached.fetched.value();

        if (_sound_track.origin == object_sound_track_origin::path) {
            return recipe_object_sound_track_path { cached.origin_path.value() };
        }

        // else if (_sound_track.origin == object_sound_track_origin::data) {
        //     return recipe_object_sound_track_data { data_audio { _sound_track.sample_rate, std::vector<float32>(_sound_track.samples_count) } };
        // }
        // BROKEN

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    assets_cell<object_sound_track>* apply_recipe(manager_assets& objects, assets_buffer<object_sound_track>& cached_vector, recipe_object_sound_track& recipe)
    {
        return std::visit([&](auto& value) -> assets_cell<object_sound_track>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_sound_track_path>) {
                return &fetch(objects, cached_vector, value.path);

            // } else if constexpr (std::is_same_v<RecipeType, recipe_object_audio_data>) {
            //     return cached_vector.create_cell(
            //         container_async<object_audio>(
            //             object_audio(std::move(value.data))));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }
}
}
