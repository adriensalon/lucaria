#pragma once

#include <lucaria/bin/event_track_data.hpp>
#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>

namespace lucaria {
namespace detail {

	struct motion_system;

    struct event_track_implementation {
        LUCARIA_DELETE_DEFAULT(event_track_implementation)
        event_track_implementation(const event_track_implementation& other) = delete;
        event_track_implementation& operator=(const event_track_implementation& other) = delete;
        event_track_implementation(event_track_implementation&& other) = default;
        event_track_implementation& operator=(event_track_implementation&& other) = default;

        event_track_implementation(const std::vector<char>& bytes);
        event_track_implementation(event_track_data&& data);

        event_track_data data;
    };

}

/// @brief Represents an event track on the host. Can be created from an event track file or from empty data.
struct event_track_object {
    event_track_object() = default;
    event_track_object(const event_track_object& other) = default;
    event_track_object& operator=(const event_track_object& other) = default;
    event_track_object(event_track_object&& other) = default;
    event_track_object& operator=(event_track_object&& other) = default;

    static event_track_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::resource_container<detail::event_track_implementation>* _resource = nullptr;
    explicit event_track_object(detail::resource_container<detail::event_track_implementation>* resource);
    friend struct detail::motion_system;
    friend struct animator_component;
};

}
