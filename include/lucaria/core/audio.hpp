#pragma once

#include <lucaria/common/audio_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents runtime audio on the host
struct audio {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(audio)
    audio(const audio& other) = delete;
    audio& operator=(const audio& other) = delete;
    audio(audio&& other) = default;
    audio& operator=(audio&& other) = default;

    /// @brief Creates audio from data
    /// @param data to create from
    audio(audio_data&& data);

    /// @brief Creates audio from bytes synchronously
    /// @param data_bytes bytes to load from
    audio(const std::vector<char>& data_bytes);

    /// @brief Loads audio from a file synchronously
    /// @param data_path path to load from
    audio(const std::filesystem::path& data_path);

    audio_data data;
};

/// @brief Loads audio from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<audio> fetch_audio(const std::filesystem::path& data_path);

}
