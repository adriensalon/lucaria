#pragma once

#include <filesystem>

void execute_woff2_compress(const std::filesystem::path& woff2_compress_exe, const std::filesystem::path& input_path, const std::filesystem::path& output_path);