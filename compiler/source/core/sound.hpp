#pragma once

#include <filesystem>

#include <glue/json.hpp>

namespace webgame {
namespace compiler {

    void compile_sound(const json::value& json_root, const std::filesystem::path& output_path);

}
}