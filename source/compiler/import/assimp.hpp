#pragma once

#include <filesystem>
#include <optional>

#include <lucaria/common/geometry.hpp>

bool assimp_has_skeleton(const std::filesystem::path& assimp_path);
geometry_data import_assimp(const std::filesystem::path& assimp_path, const std::optional<std::filesystem::path>& skeleton_path);
