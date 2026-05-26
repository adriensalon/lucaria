#pragma once

#include <filesystem>
#include <optional>

#include <lucaria/bin/data_geometry.hpp>

bool assimp_has_skeleton(const std::filesystem::path& assimp_path);
lucaria::data_geometry import_assimp(const std::filesystem::path& assimp_path, const std::optional<std::filesystem::path>& skeleton_path);
