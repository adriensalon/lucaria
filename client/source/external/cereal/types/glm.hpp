#pragma once

#include <glm/glm.hpp>

namespace glm {
    
template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec2& value)
{
    archive(value.x);
    archive(value.y);
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec3& value)
{
    archive(value.x);
    archive(value.y);
    archive(value.z);
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec4& value)
{
    archive(value.x);
    archive(value.y);
    archive(value.z);
    archive(value.w);
}

}