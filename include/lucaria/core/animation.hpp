#pragma once

#include <cereal/types/variant.hpp>
#include <ozz/animation/runtime/animation.h>

#include <lucaria/bin/path_data.hpp>
#include <lucaria/core/refcount.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>

namespace lucaria {
namespace detail {

    struct motion_system;

    enum struct animation_origin {
        path
    };

    struct animation_implementation {
        LUCARIA_DELETE_DEFAULT(animation_implementation)
        animation_implementation(const animation_implementation& other) = delete;
        animation_implementation& operator=(const animation_implementation& other) = delete;
        animation_implementation(animation_implementation&& other) = default;
        animation_implementation& operator=(animation_implementation&& other) = default;

        animation_implementation(const std::vector<char>& bytes);
        animation_implementation(ozz::animation::Animation&& animation);

        animation_origin origin;
        ozz::animation::Animation animation;
    };

    struct animation_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    using animation_recipe = std::variant<animation_path_recipe>;

    [[nodiscard]] animation_recipe make_recipe(const implementation_container<animation_implementation>& container);
    [[nodiscard]] implementation_container<animation_implementation>& apply_recipe(implementation_manager<animation_implementation>& manager, animation_recipe&& recipe);

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
    detail::implementation_manager<detail::animation_implementation>* _manager = nullptr;
    detail::implementation_container<detail::animation_implementation>* _resource = nullptr;
    explicit animation_object(detail::implementation_container<detail::animation_implementation>* cell);
    friend struct detail::motion_system;
    friend struct animator_component;
};

}
