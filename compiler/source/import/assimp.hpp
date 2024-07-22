#pragma once

#include <filesystem>
#include <optional>

#include <data/geometry.hpp>

struct imported_assimp_data {
    geometry_data mesh_geometry;
    std::optional<geometry_data> armature_geometry = std::nullopt;
};

imported_assimp_data import_assimp(const std::filesystem::path& assimp_path);
