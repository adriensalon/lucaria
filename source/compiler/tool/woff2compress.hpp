#pragma once

#include <filesystem>

void execute_woff2compress(const std::filesystem::path& woff2compress_exe, const std::filesystem::path& input_path, const std::filesystem::path& output_path);