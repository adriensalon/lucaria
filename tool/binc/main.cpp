#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <binc/import_assimp.hpp>
#include <binc/import_json.hpp>
#include <binc/import_manifest.hpp>
#include <binc/import_stb.hpp>
#include <binc/compile_bin.hpp>
#include <binc/compile_etcpak.hpp>
#include <binc/compile_geometry.hpp>
#include <binc/compile_gltf2ozz.hpp>
#include <binc/compile_oggenc.hpp>
#include <binc/compile_woff2enc.hpp>

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
    std::cout << "Usage: lucaria_binc -i <input-dir> -o <output-dir> [-p <platform>]" << std::endl;
    std::cout << "                         [-m <manifest-json>]" << std::endl;
    std::cout << "Platforms: all, win32, linux, web, android, psp" << std::endl;
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
    if (std::filesystem::exists(_output_dir) && !std::filesystem::is_directory(_output_dir)) {
        std::cout << "The path provided with option -o must be a directory" << std::endl;
        std::terminate();
    }
    std::filesystem::create_directories(_output_dir);
    std::cout << "-- Assets output directory at " << _output_dir << std::endl;
    return _output_dir;
}

asset_profiles process_platform_command(const commands_map& commands)
{
    if (commands.find("-p") == commands.end()) {
        std::cout << "-- Assets platform all" << std::endl;
        return asset_profiles { true, true, true, 2048, 0 };
    }
    if (commands.at("-p").size() != 1) {
        std::cout << "Only one platform must be provided with option -p" << std::endl;
        std::terminate();
    }
    const std::string _platform = commands.at("-p").at(0);
    std::cout << "-- Assets platform " << _platform << std::endl;
    if (_platform == "all") {
        return asset_profiles { true, true, true, 2048, 0 };
    }
    if (_platform == "psp") {
        return asset_profiles { true, false, true, 256, 8192 };
    }
    if (_platform == "win32" || _platform == "linux") {
        return asset_profiles { true, false, true, 2048, 0 };
    }
    if (_platform == "android") {
        return asset_profiles { true, true, false, 1024, 65535 };
    }
    if (_platform == "web") {
        return asset_profiles { true, true, true, 1024, 65535 };
    }
    std::cout << "Unknown platform '" << _platform << "' provided with option -p" << std::endl;
    std::terminate();
}

std::filesystem::path texture_output_path(const std::filesystem::path& output_file, const std::string& suffix)
{
    return output_file.parent_path() / (output_file.stem().string() + ".lod0" + suffix + ".bin");
}

std::optional<std::filesystem::path> process_manifest_command(const commands_map& commands)
{
    if (commands.find("-m") == commands.end()) {
        return std::nullopt;
    }
    if (commands.at("-m").size() != 1) {
        std::cout << "Only one manifest path must be provided with option -m" << std::endl;
        std::terminate();
    }
    std::filesystem::path _manifest_path = commands.at("-m").at(0);
    if (std::filesystem::exists(_manifest_path) && !std::filesystem::is_regular_file(_manifest_path)) {
        std::cout << "The path provided with option -m must be a file" << std::endl;
        std::terminate();
    }
    std::cout << "-- Assets manifest at " << _manifest_path << std::endl;
    return _manifest_path;
}

void iterate_recursive(const std::vector<std::string>& ignore_patterns, const std::filesystem::path base_dir, const std::function<bool(const std::filesystem::path&)> callback)
{
    try {
        for (const std::filesystem::directory_entry& _entry : std::filesystem::recursive_directory_iterator(base_dir)) {
            if (std::filesystem::is_regular_file(_entry.status())) {
                if (detail::matches_regex(_entry.path(), ignore_patterns)) {
                    continue;
                }
                if (callback(_entry.path())) {
                    std::cout << "   Compiled " << _entry.path() << std::endl;
                } else {
                    std::cout << "   Skipped " << _entry.path() << std::endl;
                }
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

void compile_resource(const asset_profiles profiles, const std::filesystem::path& input_file, const std::filesystem::path& output_file)
{
    const std::string _extension = input_file.extension().generic_string();
    if (_extension == ".ttf") {
        execute_woff2compress(input_file, output_file);

    } else if (_extension == ".glb" || _extension == ".gltf") {
        const bool _has_skeleton = assimp_has_skeleton(input_file);
        if (_has_skeleton) {
            const std::filesystem::path _skeleton_path = output_file.parent_path() / (input_file.stem().string() + "_skeleton.bin");
            execute_gltf2ozz(input_file, output_file.parent_path());
            export_binary(compile_geometry_lod0(import_assimp(input_file, _skeleton_path), profiles.geometry_max_vertices), geometry_output_path(output_file));
        } else {
            export_binary(compile_geometry_lod0(import_assimp(input_file, std::nullopt), profiles.geometry_max_vertices), geometry_output_path(output_file));
        }

    } else if (_extension == ".jpg" || _extension == ".png" || _extension == ".bmp") {
        const lucaria::data_image _image = import_stb(input_file, profiles.texture_size);
        if (profiles.raw) {
            export_binary(_image, texture_output_path(output_file, ""));
        }
        if (_extension == ".png") {
            const std::filesystem::path _resized_path = output_file.string() + ".resize.tmp.png";
            write_stb_png(_image, _resized_path);
            if (profiles.etc) {
                execute_etcpak(etcpak_mode::etc, _resized_path, texture_output_path(output_file, ".etc"));
            }
            if (profiles.s3tc) {
                execute_etcpak(etcpak_mode::s3tc, _resized_path, texture_output_path(output_file, ".s3tc"));
            }
            std::filesystem::remove(_resized_path);
        } else {
            std::cout << "   Not exporting to etc/s3tc because image file must be .png" << std::endl;
        }

    } else if (_extension == ".wav" || _extension == ".aiff") {
        execute_oggenc(input_file, output_file.parent_path() / (output_file.stem().string() + ".ogg.bin"));

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
    detail::commands_map _commands = detail::extract_args(argc, argv);
    if (detail::process_help_command(_commands)) {
        return 0;
    }
    std::filesystem::path _input_dir = detail::process_input_command(_commands);
    std::filesystem::path _output_dir = detail::process_output_command(_commands);
    detail::asset_profiles _profiles = detail::process_platform_command(_commands);
    std::optional<std::filesystem::path> _manifest_path = detail::process_manifest_command(_commands);
    detail::asset_manifest _manifest = detail::load_manifest(_manifest_path);
    std::unordered_set<std::string> _seen_inputs;
    const std::string _profile_key = detail::profile_key(_profiles);
    std::vector<std::string> _patterns = detail::get_ignore_codes(_input_dir);

    detail::iterate_recursive(_patterns, _input_dir, [&](const std::filesystem::path& _input_file) {
        std::cout << std::endl << "   Processing " << _input_file << std::endl;
        std::filesystem::path _relative_input_file = detail::substract_relative(_input_dir, _input_file);
        std::filesystem::path _relative_output_file = _relative_input_file;
        _relative_output_file.replace_extension(".bin");
        std::filesystem::path _output_file = _output_dir / _relative_output_file;
        std::filesystem::path _parent_path = _output_file.parent_path();
        if (!std::filesystem::exists(_parent_path)) {
            std::filesystem::create_directories(_parent_path);
        }
        const std::string _input_manifest_path = detail::manifest_path(_input_file);
        const std::int64_t _input_timestamp = detail::file_timestamp(_input_file);
        const std::vector<std::filesystem::path> _expected_outputs = detail::expected_outputs(_profiles, _input_file, _output_file);
        _seen_inputs.emplace(_input_manifest_path);
        if (_manifest_path && detail::is_up_to_date(_manifest, _input_manifest_path, _input_timestamp, _profile_key, _expected_outputs)) {
            return false;
        }
        detail::compile_resource(_profiles, _input_file, _output_file);
        if (_manifest_path) {
            _manifest[_input_manifest_path] = detail::asset_manifest_entry {
                _input_timestamp,
                _profile_key,
                detail::manifest_outputs(_expected_outputs)
            };
        }
        return true;
    });
    if (_manifest_path) {
        for (auto _entry = _manifest.begin(); _entry != _manifest.end();) {
            if (_seen_inputs.find(_entry->first) == _seen_inputs.end()) {
                _entry = _manifest.erase(_entry);
            } else {
                ++_entry;
            }
        }
        detail::write_manifest(_manifest_path, _manifest);
    }
    return 0;
}
