#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "import/assimp.hpp"
#include "import/json.hpp"
#include "import/stb.hpp"
#include "import/text.hpp"

#include "export/binary.hpp"

#include "tool/etcpak.hpp"
#include "tool/gltf2ozz.hpp"
#include "tool/oggenc.hpp"
#include "tool/woff2compress.hpp"

namespace detail {

using commands_map = std::unordered_map<std::string, std::vector<std::string>>;

std::vector<std::string> get_ignore_codes(const std::filesystem::path& input_dir)
{
    std::filesystem::path _assetignore_path = input_dir / ".assetignore";
    std::vector<std::string> _ignore_codes;
    if (std::filesystem::exists(_assetignore_path)) {
        std::ifstream _assetignore_file(_assetignore_path);
        if (!_assetignore_file) {
            std::cout << "Could not open assetignore file" << std::endl;
            std::terminate();
        }
        std::string _line;
        while (std::getline(_assetignore_file, _line)) {
            _ignore_codes.emplace_back(_line);
        }
    }
    return _ignore_codes;
}

bool matches_regex(const std::filesystem::path& path, const std::string& pattern)
{
    try {
        std::regex _pattern(pattern);
        return std::regex_match(path.string(), _pattern);
    } catch (const std::regex_error& _error) {
        std::cout << "Invalid regex " << _error.what() << std::endl;
        std::terminate();
    }
}

bool matches_regex(const std::filesystem::path& path, const std::vector<std::string>& patterns)
{
    for (const std::string& _pattern : patterns) {
        if (matches_regex(path, _pattern)) {
            return true;
        }
    }
    return false;
}

commands_map extract_args(int argc, char* argv[])
{
    commands_map _commands;
    std::string _current;
    for (int _index = 1; _index < argc; ++_index) {
        std::string arg = argv[_index];
        if (arg[0] == '-') {
            _current = arg;
            if (_commands.find(_current) == _commands.end()) {
                _commands[_current] = std::vector<std::string>();
            }
        } else if (!_current.empty()) {
            _commands[_current].push_back(arg);
        } else {
            std::cout << "Value encountered without a command '" << arg << "'" << std::endl;
            std::terminate();
        }
    }
    return _commands;
}

bool process_help_command(const commands_map& commands)
{
    if (commands.find("-h") == commands.end()) {
        return false;
    }
    std::cout << "HELP DISPLAY" << std::endl;
    // TODO
    return true;
}

std::filesystem::path process_input_command(const commands_map& commands)
{
    if (commands.find("-i") == commands.end()) {
        std::cout << "Command -i must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-i").size() != 1) {
        std::cout << "Only one directory must be provided with option -i" << std::endl;
        std::terminate();
    }
    std::filesystem::path _input_dir = commands.at("-i").at(0);
    if (!std::filesystem::is_directory(_input_dir)) {
        std::cout << "The path provided with option -i must be an existing directory" << std::endl;
        std::terminate();
    }
    std::cout << "-- Assets input directory at " << _input_dir << std::endl;
    return _input_dir;
}

std::filesystem::path process_output_command(const commands_map& commands)
{
    if (commands.find("-o") == commands.end()) {
        std::cout << "Command -o must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-o").size() != 1) {
        std::cout << "Only one directory must be provided with option -o" << std::endl;
        std::terminate();
    }
    std::filesystem::path _output_dir = commands.at("-o").at(0);
    if (!std::filesystem::is_directory(_output_dir)) {
        std::cout << "The path provided with option -o must be an existing directory" << std::endl;
        std::terminate();
    }
    std::cout << "-- Assets output directory at " << _output_dir << std::endl;
    return _output_dir;
}

void iterate_recursive(const std::vector<std::string>& ignore_patterns, const std::filesystem::path base_dir, const std::function<void(const std::filesystem::path&)> callback)
{
    try {
        for (const std::filesystem::directory_entry& _entry : std::filesystem::recursive_directory_iterator(base_dir)) {
            if (std::filesystem::is_regular_file(_entry.status())) {
                if (detail::matches_regex(_entry.path(), ignore_patterns)) {
                    continue;
                }
                callback(_entry.path());
                std::cout << "   Compiled " << _entry.path() << std::endl;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Filesystem error " << e.what() << std::endl;
        std::terminate();
    } catch (const std::exception& e) {
        std::cout << "General exception " << e.what() << std::endl;
        std::terminate();
    }
}

std::filesystem::path substract_relative(const std::filesystem::path base_dir, const std::filesystem::path file_path)
{
    try {
        if (std::filesystem::exists(base_dir) && std::filesystem::exists(file_path) && std::filesystem::is_directory(base_dir)) {
            if (file_path.string().find(base_dir.string()) == 0) {
                return std::filesystem::relative(file_path, base_dir);
            } else {
                std::cout << "The target path is not within the base directory" << std::endl;
                std::terminate();
            }
        } else {
            std::cout << "Base directory or target path does not exist" << std::endl;
            std::terminate();
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "Filesystem error " << e.what() << std::endl;
        std::terminate();
    } catch (const std::exception& e) {
        std::cout << "General exception " << e.what() << std::endl;
        std::terminate();
    }
}

void compile_resource(const std::filesystem::path& input_file, const std::filesystem::path& output_file)
{
    const std::string _extension = input_file.extension().generic_string();
    if (_extension == ".ttf") {
        execute_woff2compress(input_file, output_file);

    } else if (_extension == ".glb" || _extension == ".gltf") {
        if (assimp_has_skeleton(input_file)) {
            const std::filesystem::path _skeleton_path = output_file.parent_path() / (input_file.stem().string() + "_skeleton.bin");
            execute_gltf2ozz(input_file, output_file.parent_path());
            export_binary(import_assimp(input_file, _skeleton_path), output_file);
        } else {
            export_binary(import_assimp(input_file, std::nullopt), output_file);
        }

    } else if (_extension == ".jpg" || _extension == ".png" || _extension == ".bmp") {
        export_binary(import_stb(input_file), output_file);
        if (_extension == ".png") {
            execute_etcpak(etcpak_mode::etc, input_file, output_file.parent_path() / (output_file.stem().string() + "_etc.bin"));
            execute_etcpak(etcpak_mode::s3tc, input_file, output_file.parent_path() / (output_file.stem().string() + "_s3tc.bin"));
        } else {
            std::cout << "   Not exporting to etc/s3tc because image file must be .png" << std::endl;
        }

    } else if (_extension == ".glsl" || _extension == ".txt" || _extension == ".vert" || _extension == ".frag") {
        export_binary(import_text(input_file), output_file);

    } else if (_extension == ".wav" || _extension == ".aiff") {
        execute_oggenc(input_file, output_file);

    } else if (_extension == ".evtt") {
        export_binary(import_event_track(input_file), output_file);

    } else {
        std::cout << "Invalid input file with extension " << _extension << std::endl;
        // std::terminate();
    }
}

}

int main(int argc, char* argv[])
{
	// lucaria::detail::run_tracy_profiler_window("127.0.0.1", 8086);
    detail::commands_map _commands = detail::extract_args(argc, argv);
    if (detail::process_help_command(_commands)) {
        return 0;
    }
    std::filesystem::path _input_dir = detail::process_input_command(_commands);
    std::filesystem::path _output_dir = detail::process_output_command(_commands);
    std::vector<std::string> _patterns = detail::get_ignore_codes(_input_dir);

    detail::iterate_recursive(_patterns, _input_dir, [&](const std::filesystem::path& _input_file) {
        std::cout << std::endl;
        std::filesystem::path _relative_input_file = detail::substract_relative(_input_dir, _input_file);
        std::filesystem::path _relative_output_file = _relative_input_file;
        _relative_output_file.replace_extension(".bin");
        std::filesystem::path _output_file = _output_dir / _relative_output_file;
        std::filesystem::path _parent_path = _output_file.parent_path();
        if (!std::filesystem::exists(_parent_path)) {
            std::filesystem::create_directory(_parent_path);
        }
        detail::compile_resource(_input_file, _output_file);
    });
    return 0;
}
