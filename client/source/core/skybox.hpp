#pragma once

#include <optional>

#include <core/cubemap.hpp>
#include <core/program.hpp>

/// @brief Represents a GPU skybox managed by the application
struct skybox_ref {

    /// @brief Default constructor is not allowed because this object must be created from data
    skybox_ref() = delete;

    /// @brief 
    /// @param cubemap 
    /// @param fragment 
    skybox_ref(cubemap_ref&& cubemap, const std::optional<shader_data>& fragment = std::nullopt);
    
    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other 
    skybox_ref(const skybox_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other 
    /// @return 
    skybox_ref& operator=(const skybox_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other 
    skybox_ref(skybox_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other 
    /// @return 
    skybox_ref& operator=(skybox_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~skybox_ref();

private:
    cubemap_ref _cubemap;
    program_ref _program;
};