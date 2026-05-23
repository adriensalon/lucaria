#pragma once

#include <lucaria/bin/container_types.hpp>
#include <lucaria/bin/math_types.hpp>

namespace lucaria {

struct geometry_data {
    uint32 vertices_count = 0;
    std::vector<float32x3> positions = {};
    std::vector<float32x4> colors = {};
    std::vector<float32x3> normals = {};
    std::vector<float32x3> tangents = {};
    std::vector<float32x3> bitangents = {};
    std::vector<float32x2> texcoords = {};
    std::vector<int32x4> bones = {};
    std::vector<float32x4> weights = {};
    std::vector<uint32x3> indices = {};
    std::vector<float32x4x4> invposes = {};
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("vertices_count", vertices_count));
        archive(cereal::make_nvp("positions", positions));
        archive(cereal::make_nvp("colors", colors));
        archive(cereal::make_nvp("normals", normals));
        archive(cereal::make_nvp("tangents", tangents));
        archive(cereal::make_nvp("bitangents", bitangents));
        archive(cereal::make_nvp("texcoords", texcoords));
        archive(cereal::make_nvp("bones", bones));
        archive(cereal::make_nvp("weights", weights));
        archive(cereal::make_nvp("indices", indices));
        archive(cereal::make_nvp("invposes", invposes));
    }
};

}
