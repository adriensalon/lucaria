#pragma once

#include <filesystem>

void execute_oggenc(const std::filesystem::path& executable_path, const std::filesystem::path& input_path, const std::filesystem::path& output_directory);
