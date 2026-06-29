#pragma once

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace glm {

template <typename archive_type_t, glm::length_t length, typename value_type_t, glm::qualifier qualifier>
void serialize(archive_type_t& archive, glm::vec<length, value_type_t, qualifier>& value)
{
    archive(cereal::make_nvp("x", value.x));
    if constexpr (length >= 2) {
        archive(cereal::make_nvp("y", value.y));
    }
    if constexpr (length >= 3) {
        archive(cereal::make_nvp("z", value.z));
    }
    if constexpr (length >= 4) {
        archive(cereal::make_nvp("w", value.w));
    }
}

template <typename archive_type_t, typename value_type_t, glm::qualifier qualifier>
void serialize(archive_type_t& archive, glm::mat<4, 4, value_type_t, qualifier>& value)
{
    archive(cereal::make_nvp("column0", value[0]));
    archive(cereal::make_nvp("column1", value[1]));
    archive(cereal::make_nvp("column2", value[2]));
    archive(cereal::make_nvp("column3", value[3]));
}

template <typename archive_type_t, typename value_type_t, glm::qualifier qualifier>
void serialize(archive_type_t& archive, glm::qua<value_type_t, qualifier>& value)
{
    archive(cereal::make_nvp("x", value.x));
    archive(cereal::make_nvp("y", value.y));
    archive(cereal::make_nvp("z", value.z));
    archive(cereal::make_nvp("w", value.w));
}

}

namespace lucaria {

// scalar
using int16 = glm::int16;
using int32 = glm::int32;

using uint8 = glm::uint8;
using uint16 = glm::uint16;
using uint32 = glm::uint32;
using uint64 = glm::uint64;

using float32 = glm::float32;
using float64 = glm::float64;

// small vectors
using int32x2 = glm::vec<2, int32>;
using int32x3 = glm::vec<3, int32>;
using int32x4 = glm::vec<4, int32>;
using uint32x2 = glm::vec<2, uint32>;
using uint32x3 = glm::vec<3, uint32>;
using uint32x4 = glm::vec<4, uint32>;
using float32x2 = glm::vec<2, float32>;
using float32x3 = glm::vec<3, float32>;
using float32x4 = glm::vec<4, float32>;
using float64x2 = glm::vec<2, float64>;
using float64x3 = glm::vec<3, float64>;
using float64x4 = glm::vec<4, float64>;
using float32quat = glm::quat;

// matrices
using int32x2x2 = glm::mat<2, 2, int32>;
using int32x3x3 = glm::mat<3, 3, int32>;
using int32x4x4 = glm::mat<4, 4, int32>;
using float32x2x2 = glm::mat<2, 2, float32>;
using float32x3x3 = glm::mat<3, 3, float32>;
using float32x4x4 = glm::mat<4, 4, float32>;
using float64x2x2 = glm::mat<2, 2, float64>;
using float64x3x3 = glm::mat<3, 3, float64>;
using float64x4x4 = glm::mat<4, 4, float64>;

}
