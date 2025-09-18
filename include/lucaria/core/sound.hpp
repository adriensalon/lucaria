#pragma once

#include <filesystem>
#include <future>

#include <glm/glm.hpp>

#include <lucaria/common/audio.hpp>

namespace lucaria {

struct sound_ref {
    sound_ref() = delete;
    sound_ref(const sound_ref& other) = delete;
    sound_ref& operator=(const sound_ref& other) = delete;
    sound_ref(sound_ref&& other);
    sound_ref& operator=(sound_ref&& other);
    ~sound_ref();

    sound_ref(const audio_data& data);
    glm::uint get_id() const;

private:
    bool _is_instanced;
    glm::uint _buffer_id;
};

audio_data load_compressed_audio_data(const std::vector<char>& audio_stream);
std::shared_future<std::shared_ptr<sound_ref>> fetch_sound(const std::filesystem::path& audio_path);
void clear_sound_fetches();

}
