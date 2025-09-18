#pragma once

#include <filesystem>
#include <vector>

#include <glm/glm.hpp>

namespace lucaria {

struct uvec2_hash {
    std::size_t operator()(const glm::uvec2& vec) const;
};

struct vec3_hash {
    std::size_t operator()(const glm::vec3& vec) const;
};

struct path_vector_hash {
    std::size_t operator()(const std::vector<std::filesystem::path>& paths) const;
};

}
