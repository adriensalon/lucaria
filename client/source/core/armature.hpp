#pragma once

#include <filesystem>
#include <future>

#include <glm/glm.hpp>

#include <data/armature.hpp>

struct armature_ref {
    armature_ref() = delete;
    armature_ref(const armature_ref& other) = delete;
    armature_ref& operator=(const armature_ref& other) = delete;
    armature_ref(armature_ref&& other) = default;
    armature_ref& operator=(armature_ref&& other) = default;

    /// @brief 
    /// @param data 
    armature_ref(const armature_data& data);
    
    /// @brief 
    /// @return 
    glm::uint get_count() const;

private:
    glm::uint count;
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec4> weights = {};
    std::vector<glm::uvec4> bones = {};
};