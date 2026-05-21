#pragma once

#include <variant>

#include <ozz/animation/runtime/track.h>
#include <cereal/types/variant.hpp>

#include <lucaria/bin/path_data.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>


namespace lucaria {
namespace detail {

    struct motion_system;

    enum struct motion_track_origin {
        path
    };

    struct motion_track_implementation {
        LUCARIA_DELETE_DEFAULT(motion_track_implementation)
        motion_track_implementation(const motion_track_implementation& other) = delete;
        motion_track_implementation& operator=(const motion_track_implementation& other) = delete;
        motion_track_implementation(motion_track_implementation&& other) = default;
        motion_track_implementation& operator=(motion_track_implementation&& other) = default;

        motion_track_implementation(const std::vector<char>& bytes);
        motion_track_implementation(ozz::animation::Float3Track&& translation_track, ozz::animation::QuaternionTrack&& rotation_track);
        [[nodiscard]] float32x3 get_total_translation() const;

        motion_track_origin origin;
        ozz::animation::Float3Track translation_track;
        ozz::animation::QuaternionTrack rotation_track;
    };

    struct motion_track_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

	using motion_track_recipe = std::variant<motion_track_path_recipe>;

	[[nodiscard]] motion_track_recipe make_recipe(const implementation_container<motion_track_implementation>& container);
}

/// @brief Represents a motion track on the host. Can be created from a motion track file or from empty data.
struct motion_track_object {
    motion_track_object() = default;
    motion_track_object(const motion_track_object& other) = default;
    motion_track_object& operator=(const motion_track_object& other) = default;
    motion_track_object(motion_track_object&& other) = default;
    motion_track_object& operator=(motion_track_object&& other) = default;

    static motion_track_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::implementation_container<detail::motion_track_implementation>* _resource = nullptr;
    explicit motion_track_object(detail::implementation_container<detail::motion_track_implementation>* resource);
    friend struct detail::motion_system;
    friend struct animator_component;
};

}
