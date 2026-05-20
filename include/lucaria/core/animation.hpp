#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/refcount.hpp>

namespace lucaria {
namespace detail {

	struct motion_system;

    struct animation_implementation {
        LUCARIA_DELETE_DEFAULT(animation_implementation)
        animation_implementation(const animation_implementation& other) = delete;
        animation_implementation& operator=(const animation_implementation& other) = delete;
        animation_implementation(animation_implementation&& other) = default;
        animation_implementation& operator=(animation_implementation&& other) = default;

        animation_implementation(const std::vector<char>& bytes);
        animation_implementation(ozz::animation::Animation&& animation);

        ozz::animation::Animation animation;
    };
	
}

/// @brief Represents an animation on the host. Can be created from an animation file or from empty data.
struct animation_object {
    animation_object() = default;
    animation_object(const animation_object& other) = default;
    animation_object& operator=(const animation_object& other) = default;
    animation_object(animation_object&& other) = default;
    animation_object& operator=(animation_object&& other) = default;
	~animation_object();

    static animation_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
	detail::refcount_flag _refcount = {};
    detail::resource_manager<detail::animation_implementation>* _manager = nullptr;
    detail::resource_container<detail::animation_implementation>* _resource = nullptr;
    explicit animation_object(detail::resource_container<detail::animation_implementation>* cell);
    friend struct detail::motion_system;
    friend struct animator_component;
};

}
