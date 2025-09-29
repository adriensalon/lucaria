#pragma once

#include <lucaria/core/audio.hpp>

namespace lucaria {

struct sound {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(sound)
    sound(const sound& other) = delete;
    sound& operator=(const sound& other) = delete;
    sound(sound&& other);
    sound& operator=(sound&& other);
    ~sound();

    /// @brief 
    /// @param from 
    sound(const audio& from);    
    
    [[nodiscard]] glm::uint get_handle() const;

private:
    bool _is_owning;
    glm::uint _handle;
};

/// @brief Loads a sound from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<sound> fetch_sound(const std::filesystem::path& data_path);

}
