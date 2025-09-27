#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lucaria {

struct fadein_weight {
    glm::float32 length = 1.f;

    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor);
};

struct fadeout_weight {
    glm::float32 length = 1.f;

    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor, const glm::float32 duration);
};

struct oscillate_weight {
    glm::float32 period = 1.f;

    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor);
};

}
