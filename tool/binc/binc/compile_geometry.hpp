#pragma once

#include <filesystem>

#include <lucaria/bin/data_geometry.hpp>

namespace detail {

std::filesystem::path geometry_output_path(const std::filesystem::path& output_file);
lucaria::data_geometry compile_geometry_lod0(lucaria::data_geometry geometry, lucaria::uint32 max_vertices);

}
