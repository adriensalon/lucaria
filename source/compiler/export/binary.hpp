#pragma once

#include <filesystem>

#include <lucaria/common/geometry_data.hpp>
#include <lucaria/common/image_data.hpp>
#include <lucaria/common/shader_data.hpp>
#include <lucaria/common/event_track_data.hpp>

void export_binary(const lucaria::geometry_data& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::shader_data& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::image_data& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::event_track_data& data, const std::filesystem::path& output_path);
