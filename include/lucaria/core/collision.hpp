#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace lucaria {

/// @brief
enum struct collision_layer : std::int16_t {
    layer_0 = 1 << 0,
    layer_1 = 1 << 1,
    layer_2 = 1 << 2,
    layer_3 = 1 << 3,
    layer_4 = 1 << 4,
    layer_5 = 1 << 5,
    layer_6 = 1 << 6,
    layer_7 = 1 << 7,
    layer_8 = 1 << 8,
    layer_9 = 1 << 9,
    layer_10 = 1 << 10,
    layer_11 = 1 << 11,
    layer_12 = 1 << 12,
    layer_13 = 1 << 13,
    layer_14 = 1 << 14
};

/// @brief 
struct collision {
    glm::float32 distance;
    glm::vec3 position;
    glm::vec3 normal;
};

}
