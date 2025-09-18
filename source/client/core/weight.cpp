#include <lucaria/core/weight.hpp>

namespace lucaria {

glm::float32 fadein_weight::compute_weight(const glm::float32 cursor)
{
    if (cursor >= length) {
        return 1.f;
    }
    return glm::clamp(0.5f * (1.f - glm::cos(glm::pi<glm::float32>() * cursor / length)), 0.0f, 1.0f);
}

glm::float32 fadeout_weight::compute_weight(const glm::float32 cursor, const glm::float32 duration)
{
    const glm::float32 _reversed = duration - cursor;
    if (_reversed >= length) {
        return 1.f;
    } else if (_reversed < 0.f) {
        return 0.f;
    }
    return glm::clamp(0.5f * (1.f - glm::cos(glm::pi<glm::float32>() * _reversed / length)), 0.0f, 1.0f);
}

glm::float32 oscillate_weight::compute_weight(const glm::float32 cursor)
{
    return glm::clamp(0.5f * (1.f - glm::cos(2.f * glm::pi<glm::float32>() * cursor / period)), 0.0f, 1.0f);
}

}