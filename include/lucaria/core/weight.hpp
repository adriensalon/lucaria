#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lucaria {

/// @brief Weight fadein helper for managing tweens across frames
struct fadein_weight {
    glm::float32 length = 1.f;

    /// @brief Computes the current weight from the selected ratio
    /// @param cursor the ratio to compute from, clamped to [0, 1]
    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor);
};

/// @brief Weight fadeout helper for managing tweens across frames
struct fadeout_weight {
    glm::float32 length = 1.f;

    /// @brief Computes the current weight from the selected ratio
    /// @param cursor the ratio to compute from, clamped to [0, 1]
    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor, const glm::float32 duration);
};

/// @brief Weight oscillate helper for managing tweens across frames
struct oscillate_weight {
    glm::float32 period = 1.f;

    /// @brief Computes the current weight from the selected ratio
    /// @param cursor the ratio to compute from, clamped to [0, 1]
    [[nodiscard]] glm::float32 compute_weight(const glm::float32 cursor);
};

}
