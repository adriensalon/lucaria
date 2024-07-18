#pragma once

#include <glm/glm.hpp>

struct uvec2_hash {
    std::size_t operator()(const glm::uvec2& vec) const
    {
        std::size_t _h1 = std::hash<glm::uint> {}(vec.x);
        std::size_t _h2 = std::hash<glm::uint> {}(vec.y);
        return _h1 ^ (_h2 << 1);
    }
};

struct vec3_hash {
    inline std::size_t operator()(const glm::vec3& vec) const
    {
        std::size_t _h1 = std::hash<glm::float32> {}(vec.x);
        std::size_t _h2 = std::hash<glm::float32> {}(vec.y);
        std::size_t _h3 = std::hash<glm::float32> {}(vec.z);
        return _h1 ^ (_h2 << 1) ^ (_h3 << 2);
    }
};