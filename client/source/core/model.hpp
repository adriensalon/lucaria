#pragma once

#include <core/mesh.hpp>
#include <core/texture.hpp>

struct model_ref {

    /// @brief Default constructor is not allowed because this object must be created from data
    model_ref() = delete;

    /// @brief 
    /// @param mesh 
    /// @param texture 
    model_ref(mesh_ref&& mesh, texture_ref&& texture);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other the other managed object
    model_ref(const model_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other the other managed object
    /// @return the same object
    model_ref& operator=(const model_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other the other managed object
    model_ref(model_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other the other managed object
    /// @return the same object
    model_ref& operator=(model_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~model_ref();

    ///
    void draw(const program_ref& program);

    /// @brief 
    /// @return 
    mesh_ref& get_mesh();

    /// @brief 
    /// @return 
    texture_ref& get_texture();

private:
    mesh_ref mesh;
    texture_ref texture;
};