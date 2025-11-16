#pragma once

#include <ozz/animation/runtime/track.h>

#include <lucaria/common/event_track_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents a runtime motion track
struct motion_track {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(motion_track)
    motion_track(const motion_track& other) = delete;
    motion_track& operator=(const motion_track& other) = delete;
    motion_track(motion_track&& other) = default;
    motion_track& operator=(motion_track&& other) = default;

    /// @brief Loads a motion track from bytes synchronously
    /// @param data_bytes bytes to load from
    motion_track(const std::vector<char>& data_bytes);

    /// @brief Loads a motion track from a file synchronously
    /// @param data_path path to load from
    motion_track(const std::filesystem::path& data_path);

    /// @brief Returns a handle to the underlying implementation 
    [[nodiscard]] ozz::animation::Float3Track& get_translation_handle();
    
    /// @brief Returns a handle to the underlying implementation 
    [[nodiscard]] const ozz::animation::Float3Track& get_translation_handle() const;
    
    /// @brief Returns a handle to the underlying implementation 
    [[nodiscard]] ozz::animation::QuaternionTrack& get_rotation_handle();
    
    /// @brief Returns a handle to the underlying implementation 
    [[nodiscard]] const ozz::animation::QuaternionTrack& get_rotation_handle() const;
    
    /// @brief Returns a handle to the underlying implementation 
    [[nodiscard]] glm::vec3 get_total_translation() const;

private:
    ozz::animation::Float3Track _translation_handle;
    ozz::animation::QuaternionTrack _rotation_handle;
};

/// @brief Represents a runtime event track
struct event_track {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(event_track)
    event_track(const event_track& other) = delete;
    event_track& operator=(const event_track& other) = delete;
    event_track(event_track&& other) = default;
    event_track& operator=(event_track&& other) = default;

    /// @brief Loads a motion track from bytes synchronously
    /// @param data_bytes bytes to load from
    event_track(const std::vector<char>& data_bytes);

    /// @brief Loads a motion track from a file synchronously
    /// @param data_path path to load from
    event_track(const std::filesystem::path& data_path);

    event_track_data data;
};

/// @brief Loads a motion track from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<motion_track> fetch_motion_track(const std::filesystem::path& data_path);

/// @brief Loads an event track from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<event_track> fetch_event_track(const std::filesystem::path& data_path);

}
