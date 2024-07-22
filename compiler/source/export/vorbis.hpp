#pragma once

#include <filesystem>

#include <data/audio.hpp>

void export_vorbis(const audio_data& data, const std::filesystem::path& output_path);
