#pragma once

#include <lucaria/core/audio.hpp>

namespace lucaria {

/// @brief Represents a runtime sound
struct sound_track {
    LUCARIA_DELETE_DEFAULT(sound_track)
    sound_track(const sound_track& other) = delete;
    sound_track& operator=(const sound_track& other) = delete;
    sound_track(sound_track&& other);
    sound_track& operator=(sound_track&& other);
    ~sound_track();

    /// @brief Creates a sound from audio data
    /// @param from the audio data to create from
    sound_track(const audio& from);    
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;
    
    [[nodiscard]] glm::uint get_sample_rate() const;
    
    [[nodiscard]] glm::uint get_count() const;

private:
    bool _is_owning;
    glm::uint _handle;
    glm::uint _sample_rate;
    glm::uint _count;
};

/// @brief Loads a sound from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<sound_track> fetch_sound(const std::filesystem::path& data_path);

}
