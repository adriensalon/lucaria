#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "import/assimp.hpp"
#include "import/stb.hpp"
#include "import/text.hpp"

#include "export/binary.hpp"

#include "tool/etcpak.hpp"
#include "tool/gltf2ozz.hpp"
#include "tool/oggenc.hpp"
#include "tool/woff2compress.hpp"

namespace detail {

using commands_map = std::unordered_map<std::string, std::vector<std::string>>;

static std::filesystem::path etcpak_executable;
static std::filesystem::path gltf2ozz_executable;
static std::filesystem::path oggenc_executable;
static std::filesystem::path woff2compress_executable;

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

std::filesystem::path process_etcpak_command(const commands_map& commands)
{
    if (commands.find("-etcpak") == commands.end()) {
        std::cout << "Command -etcpak must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-etcpak").size() != 1) {
        std::cout << "Only one file must be provided with option -etcpak" << std::endl;
        std::terminate();
    }
    std::filesystem::path _etcpak_executable = commands.at("-etcpak").at(0);
    if (!std::filesystem::exists(_etcpak_executable)) {
        std::cout << "The path provided with option -etcpak must be an existing application" << std::endl;
        std::terminate();
    }
    std::cout << "-- Tool etcpak provided at " << _etcpak_executable << std::endl;
    return _etcpak_executable;
}

std::filesystem::path process_gltf2ozz_command(const commands_map& commands)
{
    if (commands.find("-gltf2ozz") == commands.end()) {
        std::cout << "Command -gltf2ozz must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-gltf2ozz").size() != 1) {
        std::cout << "Only one file must be provided with option -gltf2ozz" << std::endl;
        std::terminate();
    }
    std::filesystem::path _gltf2ozz_executable = commands.at("-gltf2ozz").at(0);
    if (!std::filesystem::exists(_gltf2ozz_executable)) {
        std::cout << "The path provided with option -gltf2ozz must be an existing application" << std::endl;
        std::terminate();
    }
    std::cout << "-- Tool gltf2ozz provided at " << _gltf2ozz_executable << std::endl;
    return _gltf2ozz_executable;
}

std::filesystem::path process_oggenc_command(const commands_map& commands)
{
    if (commands.find("-oggenc") == commands.end()) {
        std::cout << "Command -oggenc must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-oggenc").size() != 1) {
        std::cout << "Only one file must be provided with option -oggenc" << std::endl;
        std::terminate();
    }
    std::filesystem::path _oggenc_executable = commands.at("-oggenc").at(0);
    if (!std::filesystem::exists(_oggenc_executable)) {
        std::cout << "The path provided with option -oggenc must be an existing application" << std::endl;
        std::terminate();
    }
    std::cout << "-- Tool oggenc provided at " << _oggenc_executable << std::endl;
    return _oggenc_executable;
}

std::filesystem::path process_woff2compress_command(const commands_map& commands)
{
    if (commands.find("-woff2compress") == commands.end()) {
        std::cout << "Command -woff2compress must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-woff2compress").size() != 1) {
        std::cout << "Only one file must be provided with option -woff2compress" << std::endl;
        std::terminate();
    }
    std::filesystem::path _woff2compress_executable = commands.at("-woff2compress").at(0);
    if (!std::filesystem::exists(_woff2compress_executable)) {
        std::cout << "The path provided with option -woff2compress must be an existing application" << std::endl;
        std::terminate();
    }
    std::cout << "-- Tool woff2compress provided at " << _woff2compress_executable << std::endl;
    return _woff2compress_executable;
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
        execute_woff2compress(woff2compress_executable, input_file, output_file);

    } else if (_extension == ".glb" || _extension == ".gltf") {
        if (assimp_has_skeleton(input_file)) {
            const std::filesystem::path _skeleton_path = output_file.parent_path() / (input_file.stem().string() + "_skeleton.bin");
            execute_gltf2ozz(gltf2ozz_executable, input_file, output_file.parent_path());
            export_binary(import_assimp(input_file, _skeleton_path), output_file);
        } else {
            export_binary(import_assimp(input_file, std::nullopt), output_file);
        }

    } else if (_extension == ".jpg" || _extension == ".png" || _extension == ".bmp") {
        export_binary(import_stb(input_file), output_file);
        if (_extension == ".png") {
            execute_etcpak(etcpak_mode::etc, etcpak_executable, input_file, output_file.parent_path() / (output_file.stem().string() + "_etc.bin"));
            execute_etcpak(etcpak_mode::s3tc, etcpak_executable, input_file, output_file.parent_path() / (output_file.stem().string() + "_s3tc.bin"));
        } else {
            std::cout << "   Not exporting to etc/s3tc because image file must be .png" << std::endl;
        }

    } else if (_extension == ".glsl" || _extension == ".txt" || _extension == ".vert" || _extension == ".frag") {
        export_binary(import_text(input_file), output_file);

    } else if (_extension == ".wav" || _extension == ".aiff") {
        execute_oggenc(oggenc_executable, input_file, output_file.parent_path());

    } else {
        std::cout << "Invalid input file with extension " << _extension << std::endl;
        // std::terminate();
    }
}

}

int main(int argc, char* argv[])
{
    detail::commands_map _commands = detail::extract_args(argc, argv);
    if (detail::process_help_command(_commands)) {
        return 0;
    }
    std::filesystem::path _input_dir = detail::process_input_command(_commands);
    std::filesystem::path _output_dir = detail::process_output_command(_commands);
    detail::etcpak_executable = detail::process_etcpak_command(_commands);
    detail::gltf2ozz_executable = detail::process_gltf2ozz_command(_commands);
    detail::oggenc_executable = detail::process_oggenc_command(_commands);
    detail::woff2compress_executable = detail::process_woff2compress_command(_commands);
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
