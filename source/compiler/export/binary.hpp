#pragma once

#include <filesystem>

#include <lucaria/bin/geometry_data.hpp>
#include <lucaria/bin/image_data.hpp>
#include <lucaria/bin/shader_data.hpp>
#include <lucaria/bin/event_track_data.hpp>

void export_binary(const lucaria::data_geometry& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::data_shader& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::image_data& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::data_event_track& data, const std::filesystem::path& output_path);
