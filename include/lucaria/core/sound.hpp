#pragma once

#include <lucaria/core/audio.hpp>

namespace lucaria {

/// @brief Represents a runtime sound
struct sound {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(sound)
    sound(const sound& other) = delete;
    sound& operator=(const sound& other) = delete;
    sound(sound&& other);
    sound& operator=(sound&& other);
    ~sound();

    /// @brief Creates a sound from audio data
    /// @param from the audio data to create from
    sound(const audio& from);    
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] glm::uint get_handle() const;

private:
    bool _is_owning;
    glm::uint _handle;
};

/// @brief Loads a sound from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<sound> fetch_sound(const std::filesystem::path& data_path);

}
