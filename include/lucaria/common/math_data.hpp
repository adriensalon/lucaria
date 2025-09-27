#pragma once

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

namespace glm {
    
template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec2& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec3& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::ivec3& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::uvec3& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::vec4& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
    archive(cereal::make_nvp("w", value.w));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::ivec4& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
    archive(cereal::make_nvp("w", value.w));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::uvec4& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
    archive(cereal::make_nvp("w", value.w));
}

template <typename archive_type_t>
void serialize(archive_type_t& archive, glm::mat4& value)
{
    archive(cereal::make_nvp("column0", value[0]));
    archive(cereal::make_nvp("column1", value[1]));
    archive(cereal::make_nvp("column2", value[2]));
    archive(cereal::make_nvp("column3", value[3]));
}

}