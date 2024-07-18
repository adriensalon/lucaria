#pragma once

#include <glm/glm.hpp>

struct vec3_hash {
    inline std::size_t operator()(const glm::vec3& vec) const
    {
        std::size_t h1 = std::hash<float> {}(vec.x);
        std::size_t h2 = std::hash<float> {}(vec.y);
        std::size_t h3 = std::hash<float> {}(vec.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2); // Combine hashes
    }
};