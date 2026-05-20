#pragma once

#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>

namespace lucaria {
namespace detail {

	struct motion_system;

    struct skeleton_implementation {
        LUCARIA_DELETE_DEFAULT(skeleton_implementation)
        skeleton_implementation(const skeleton_implementation& other) = delete;
        skeleton_implementation& operator=(const skeleton_implementation& other) = delete;
        skeleton_implementation(skeleton_implementation&& other) = default;
        skeleton_implementation& operator=(skeleton_implementation&& other) = default;

        skeleton_implementation(const std::vector<char>& bytes);
        skeleton_implementation(ozz::animation::Skeleton&& skeleton);

        ozz::animation::Skeleton skeleton;
    };

}

struct skeleton_object {
    skeleton_object() = default;
    skeleton_object(const skeleton_object& other) = default;
    skeleton_object& operator=(const skeleton_object& other) = default;
    skeleton_object(skeleton_object&& other) = default;
    skeleton_object& operator=(skeleton_object&& other) = default;

    /// TODO GO CONTEXT
    static skeleton_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the skeleton is ready to be used
    /// @return true if the skeleton is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::resource_container<detail::skeleton_implementation>* _resource = nullptr;
    explicit skeleton_object(detail::resource_container<detail::skeleton_implementation>* resource);
    friend struct detail::motion_system;
    friend struct animator_component;
};

}
