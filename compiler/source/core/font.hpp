#pragma once

#include <filesystem>

#include <utils/json_utils.hpp>

namespace webgame {
namespace compiler {

    void compile_font(const json::value& json_root, const std::filesystem::path& output_path);

}
}