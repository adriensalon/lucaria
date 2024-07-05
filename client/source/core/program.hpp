#pragma once

#include <filesystem>

#include <glm/glm.hpp>

#include <core/cubemap.hpp>
#include <core/mesh.hpp>
#include <core/texture.hpp>
#include <data/shader.hpp>

/// @brief Represents a GPU program managed by the application
struct program_ref {
    program_ref() = delete;
    program_ref(const program_ref& other) = delete;
    program_ref& operator=(const program_ref& other) = delete;
    program_ref(program_ref&& other);
    program_ref& operator=(program_ref&& other);
    ~program_ref();

    /// @brief Loads the program from an asset data
    /// @param vertex
    /// @param fragment
    program_ref(const shader_data& vertex, const shader_data& fragment);

    /// @brief 
    void use() const;

    /// @brief 
    /// @param mesh 
    /// @param name 
    /// @param attribute 
    void bind(const std::string& name, const mesh_ref& mesh, const mesh_attribute attribute);

    /// @brief 
    /// @param cubemap 
    /// @param name 
    /// @param slot 
    void bind(const std::string& name, const cubemap_ref& cubemap, const glm::uint slot = 0) const;

    /// @brief 
    /// @param texture 
    /// @param name 
    /// @param slot 
    void bind(const std::string& name, const texture_ref& texture, const glm::uint slot = 0) const;

    /// @brief 
    /// @tparam value_t 
    /// @param name 
    /// @param value 
    template <typename value_t>
    void bind(const std::string& name, const value_t& value);

    /// @brief 
    void draw() const;

    /// @brief Gets the OpenGL id for this managed data
    /// @return the program id as a glm::uint
    glm::uint get_id() const;

private:
    glm::uint _program_id;
    glm::uint _array_id;
    glm::uint _count;
    std::unordered_map<std::string, glm::int32> _program_attributes;
    std::unordered_map<std::string, glm::int32> _program_uniforms;
    bool _must_destroy;
    inline static glm::uint _used_program_id = 0;
};

/// @brief
/// @param file
/// @return
shader_data load_shader(const std::filesystem::path& file);
