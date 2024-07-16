#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <future>
#include <unordered_map>
#include <sstream>
#include <vector>

#include <glm/glm.hpp>

#include <data/armature.hpp>

struct armature_ref {
    armature_ref() = delete;
    armature_ref(const armature_ref& other) = delete;
    armature_ref& operator=(const armature_ref& other) = delete;
    armature_ref(armature_ref&& other) = default;
    armature_ref& operator=(armature_ref&& other) = default;

    armature_ref(const armature_data& data);
    const std::vector<glm::vec3>& get_positions() const;
    const std::vector<glm::uvec4>& get_bones() const;
    const std::vector<glm::vec4>& get_weights() const;
    glm::uint get_count() const;

private:
    glm::uint _count;
    std::vector<glm::vec3> _positions;
    std::vector<glm::uvec4> _bones;
    std::vector<glm::vec4> _weights;
};

armature_data load_armature_data(std::istringstream& armature_stream);
std::shared_future<std::shared_ptr<armature_ref>> fetch_armature(const std::filesystem::path& armature_path);
