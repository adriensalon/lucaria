#pragma once

#include <filesystem>

#include <lucaria/common/geometry.hpp>
#include <lucaria/common/image.hpp>
#include <lucaria/common/shader.hpp>

void export_binary(const geometry_data& data, const std::filesystem::path& output_path);
void export_binary(const shader_data& data, const std::filesystem::path& output_path);
void export_binary(const image_data& data, const std::filesystem::path& output_path);
