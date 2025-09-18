#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct fadein_weight {
    fadein_weight() = default;
    fadein_weight(const fadein_weight& other) = default;
    fadein_weight& operator=(const fadein_weight& other) = default;
    fadein_weight(fadein_weight&& other) = default;
    fadein_weight& operator=(fadein_weight&& other) = default;

    glm::float32 length = 1.f;

    glm::float32 compute_weight(const glm::float32 cursor);
};

struct fadeout_weight {
    fadeout_weight() = default;
    fadeout_weight(const fadeout_weight& other) = default;
    fadeout_weight& operator=(const fadeout_weight& other) = default;
    fadeout_weight(fadeout_weight&& other) = default;
    fadeout_weight& operator=(fadeout_weight&& other) = default;

    glm::float32 length = 1.f;

    glm::float32 compute_weight(const glm::float32 cursor, const glm::float32 duration);
};

struct oscillate_weight {
    oscillate_weight() = default;
    oscillate_weight(const oscillate_weight& other) = default;
    oscillate_weight& operator=(const oscillate_weight& other) = default;
    oscillate_weight(oscillate_weight&& other) = default;
    oscillate_weight& operator=(oscillate_weight&& other) = default;

    glm::float32 period = 1.f;

    glm::float32 compute_weight(const glm::float32 cursor);
};