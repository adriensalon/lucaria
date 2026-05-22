#pragma once

#include <variant>

#include <cereal/types/variant.hpp>

#include <lucaria/bin/event_track_data.hpp>
#include <lucaria/bin/path_data.hpp>
#include <lucaria/core/refcount.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>

namespace lucaria {
namespace detail {

    struct motion_system;

    enum struct event_track_origin {
        path
    };

    struct event_track_implementation {
        LUCARIA_DELETE_DEFAULT(event_track_implementation)
        event_track_implementation(const event_track_implementation& other) = delete;
        event_track_implementation& operator=(const event_track_implementation& other) = delete;
        event_track_implementation(event_track_implementation&& other) = default;
        event_track_implementation& operator=(event_track_implementation&& other) = default;

        event_track_implementation(const std::vector<char>& bytes);
        event_track_implementation(event_track_data&& data);

        event_track_origin origin;
        event_track_data data;
    };

    struct event_track_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    using event_track_recipe = std::variant<event_track_path_recipe>;

    [[nodiscard]] event_track_recipe make_recipe(const implementation_container<event_track_implementation>& container);

}

/// @brief Represents an event track on the host. Can be created from an event track file or from empty data.
struct event_track_object {
    event_track_object() = default;
    event_track_object(const event_track_object& other) = default;
    event_track_object& operator=(const event_track_object& other) = default;
    event_track_object(event_track_object&& other) = default;
    event_track_object& operator=(event_track_object&& other) = default;
    ~event_track_object();

    static event_track_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::event_track_implementation>* _manager = nullptr;
    detail::implementation_container<detail::event_track_implementation>* _resource = nullptr;
    
	template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

	friend struct detail::motion_system;
    friend struct animator_component;
	friend class cereal::access;
};

}
