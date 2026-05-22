#pragma once

#include <variant>

#include <cereal/types/variant.hpp>
#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/bin/path_data.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>


namespace lucaria {
namespace detail {

    struct motion_system;

    enum struct skeleton_origin {
        path
    };

    struct skeleton_implementation {
        LUCARIA_DELETE_DEFAULT(skeleton_implementation)
        skeleton_implementation(const skeleton_implementation& other) = delete;
        skeleton_implementation& operator=(const skeleton_implementation& other) = delete;
        skeleton_implementation(skeleton_implementation&& other) = default;
        skeleton_implementation& operator=(skeleton_implementation&& other) = default;

        skeleton_implementation(const std::vector<char>& bytes);
        skeleton_implementation(ozz::animation::Skeleton&& skeleton);

        skeleton_origin origin;
        ozz::animation::Skeleton skeleton;
    };

    struct skeleton_path_recipe {
        std::filesystem::path path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

	using skeleton_recipe = std::variant<skeleton_path_recipe>;

	[[nodiscard]] skeleton_recipe make_recipe(const implementation_container<skeleton_implementation>& container);

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
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::skeleton_implementation>* _manager = nullptr;
    detail::implementation_container<detail::skeleton_implementation>* _resource = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct detail::motion_system;
    friend struct animator_component;
	friend class cereal::access;
};

}
