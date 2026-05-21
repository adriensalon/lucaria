#pragma once

#include <lucaria/bin/geometry_data.hpp>
#include <lucaria/core/workaround.hpp>
#include <lucaria/core/resource.hpp>

namespace lucaria {
namespace detail {

	struct shape_implementation;
	struct mesh_implementation;
	struct rendering_system;

    struct geometry_implementation {
        LUCARIA_DELETE_DEFAULT(geometry_implementation)
        geometry_implementation(const geometry_implementation& other) = delete;
        geometry_implementation& operator=(const geometry_implementation& other) = delete;
        geometry_implementation(geometry_implementation&& other) = default;
        geometry_implementation& operator=(geometry_implementation&& other) = default;

        geometry_implementation(const std::vector<char>& bytes);
        geometry_implementation(const mesh_implementation& mesh);
        geometry_implementation(const shape_implementation& shape);
        geometry_implementation(geometry_data&& data);

        geometry_data data;
    };

}

/// @brief Represents a geometry on the device. Can be created from a geometry file.
struct geometry_object {
    geometry_object() = default;
    geometry_object(const geometry_object& other) = default;
    geometry_object& operator=(const geometry_object& other) = default;
    geometry_object(geometry_object&& other) = default;
    geometry_object& operator=(geometry_object&& other) = default;

    static geometry_object fetch(const std::filesystem::path& path);

    /// @brief Checks if the image is ready to be used
    /// @return true if the image is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::resource_container<detail::geometry_implementation>* _resource = nullptr;
    explicit geometry_object(detail::resource_container<detail::geometry_implementation>* resource);
    friend struct detail::rendering_system;
    friend struct shape_object;
    friend struct spatial_interface_component;
};

}
