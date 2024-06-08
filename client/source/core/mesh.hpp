#pragma once

#include <filesystem>

#include <GLES3/gl3.h>

#include <data/mesh.hpp>

struct mesh_ref {
    
    /// @brief Default constructor is not allowed because this object must be created from data
    mesh_ref() = delete;

    /// @brief 
    /// @param data 
    mesh_ref(const mesh_data& data);

    /// @brief Copy constructor is not allowed because this object represents managed data
    /// @param other the other managed object
    mesh_ref(const mesh_ref& other) = delete;

    /// @brief Copy assignment is not allowed because this object represents managed data
    /// @param other the other managed object
    /// @return the same object
    mesh_ref& operator=(const mesh_ref& other) = delete;

    /// @brief Move constructor transfers ownership of the managed data
    /// @param other the other managed object
    mesh_ref(mesh_ref&& other) = default;

    /// @brief Move assignment transfers ownership of the managed data
    /// @param other the other managed object
    /// @return the same object
    mesh_ref& operator=(mesh_ref&& other) = default;

    /// @brief Destructor ensure managed data is released before destruction
    ~mesh_ref();

    /// @brief Gets the OpenGL id for this managed data
    /// @return the texture id as a GLuint
    GLuint get_id() const;

private:
    enum struct vertex_attribute {
        position,
        normal,
        texcoord,
        color 
    };
    struct vertex_buffer {
        vertex_attribute attribute;
        GLuint size;
        GLuint buffer_id;
    };
    std::vector<vertex_buffer> _vertex_buffers;
    GLuint _vertices_count;
    GLuint _vertex_array_id;
    GLuint _elements_buffer_id;
};