#pragma once

#include <filesystem>

#include <data/geometry.hpp>
#include <data/image.hpp>
#include <data/shader.hpp>

void export_binary(const geometry_data& data, const std::filesystem::path& output_path);
void export_binary(const shader_data& data, const std::filesystem::path& output_path);
void export_binary(const image_data& data, const std::filesystem::path& output_path);
