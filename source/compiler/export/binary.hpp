#pragma once

#include <filesystem>

#include <lucaria/bin/data_geometry.hpp>
#include <lucaria/bin/data_image.hpp>
#include <lucaria/bin/data_shader.hpp>
#include <lucaria/bin/data_event_track.hpp>

void export_binary(const lucaria::data_geometry& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::data_shader& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::data_image& data, const std::filesystem::path& output_path);
void export_binary(const lucaria::data_event_track& data, const std::filesystem::path& output_path);
