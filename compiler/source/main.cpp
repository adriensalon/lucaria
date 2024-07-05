#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include <core/import.hpp>

/// @brief Represents a map containing every argument for each command
using commands_map = std::unordered_map<std::string, std::vector<std::string>>;

/// @brief Extracts all commands with arguments from main parameters
/// @param argc the first parameter taken by the main function
/// @param argv the second parameter taken by the main function
/// @return the map containings all the commands
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
            std::cerr << "Value encountered without a command '" << arg << "'" << std::endl;
            std::terminate();
        }
    }
    return _commands;
}

/// @brief 
/// @param commands 
/// @return 
bool process_help_command(const commands_map& commands)
{
    if (commands.find("-h") == commands.end()) {
        return false;
    }
    std::cout << "HELP DISPLAY" << std::endl;
    // TODO
    return true;
}

/// @brief 
/// @param commands 
/// @return 
std::filesystem::path process_input_command(const commands_map& commands)
{
    if (commands.find("-i") == commands.end()) {
        std::cerr << "Command -i must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-i").size() != 1) {
        std::cerr << "Only one directory must be provided with option -i" << std::endl;
        std::terminate();
    }
    std::filesystem::path _input_dir = commands.at("-i").at(0);
    if (!std::filesystem::is_directory(_input_dir)) {        
        std::cerr << "The path provided with option -i must be an existing directory" << std::endl;
        std::terminate();
    }
    std::cout << "-- Assets input directory at " << _input_dir << std::endl;
    return _input_dir;
}

/// @brief 
/// @param commands 
/// @return 
std::filesystem::path process_output_command(const commands_map& commands)
{
    if (commands.find("-o") == commands.end()) {
        std::cerr << "Command -o must be provided" << std::endl;
        std::terminate();
    }
    if (commands.at("-o").size() != 1) {
        std::cerr << "Only one directory must be provided with option -o" << std::endl;
        std::terminate();
    }
    std::filesystem::path _output_dir = commands.at("-o").at(0);
    if (!std::filesystem::is_directory(_output_dir)) {        
        std::cerr << "The path provided with option -o must be an existing directory" << std::endl;
        std::terminate();
    }
    std::cout << "-- Assets output directory at " << _output_dir << std::endl;
    return _output_dir;
}

/// @brief 
/// @param base_dir 
/// @param callback 
void iterate_recursive(const std::filesystem::path base_dir, const std::function<void(const std::filesystem::path&)> callback)
{
    try {
        for (const std::filesystem::directory_entry& _entry : std::filesystem::recursive_directory_iterator(base_dir)) {
            if (std::filesystem::is_regular_file(_entry.status())) {
                callback(_entry.path());
                std::cout << "   Compiled " << _entry.path() << std::endl;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error " << e.what() << std::endl;
        std::terminate();
    } catch (const std::exception& e) {
        std::cerr << "General exception " << e.what() << std::endl;
        std::terminate();
    }
}

/// @brief 
/// @param base_dir 
/// @param file_path 
/// @return 
std::filesystem::path substract_relative(const std::filesystem::path base_dir, const std::filesystem::path file_path)
{
    try {
        if (std::filesystem::exists(base_dir) && std::filesystem::exists(file_path) && std::filesystem::is_directory(base_dir)) {
            if (file_path.string().find(base_dir.string()) == 0) {
                return std::filesystem::relative(file_path, base_dir);
            } else {
                std::cerr << "The target path is not within the base directory" << std::endl;
                std::terminate();
            }
        } else {
            std::cerr << "Base directory or target path does not exist" << std::endl;
            std::terminate();
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error " << e.what() << std::endl;
        std::terminate();
    } catch (const std::exception& e) {
        std::cerr << "General exception " << e.what() << std::endl;
        std::terminate();
    }
}

/// @brief 
/// @tparam resource_data_t 
/// @param data 
/// @param output 
template <typename resource_data_t>
void compile_binary_or_json(const resource_data_t& data, const std::filesystem::path& output)
{
#if LUCARIA_JSON
    std::ofstream _fstream(output);
    cereal::JSONOutputArchive _archive(_fstream);
#else
    std::ofstream _fstream(output, std::ios::binary);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
#endif
    _archive(data);
}

/// @brief 
/// @param input 
/// @param output_directory 
void copy_ozz_files(const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    std::filesystem::path _current_path = std::filesystem::current_path();
    for (const std::filesystem::directory_entry& _entry : std::filesystem::directory_iterator(_current_path)) {
        if (std::filesystem::is_regular_file(_entry.path()) && _entry.path().extension().string() == ".ozz") {
            std::filesystem::path _destination_file;
            if (_entry.path().filename().string() == "skeleton.ozz") {
                _destination_file = output_directory / (input.stem().string() + "_skeleton.bin");
            } else {
                _destination_file = output_directory / (input.stem().string() + "_animation_" + _entry.path().stem().string() + ".bin");
            }
            std::filesystem::rename(_entry.path(), _destination_file);
        }
    }
}

/// @brief 
/// @param input 
/// @param output_directory 
void compile_ozz_resources(const std::filesystem::path& input, const std::filesystem::path& output_directory)
{
    const std::string _command = std::filesystem::current_path().string() + "/compiler/gltf2ozz.exe --file=" + input.string();
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Error: gltf2ozz command failed with exit code " << _result << std::endl;
        std::terminate();
    }
    copy_ozz_files(input, output_directory);
}

/// @brief 
/// @param input_path 
/// @param output_path 
void compile_resource(const std::filesystem::path& input_file, const std::filesystem::path& output_file)
{
    const std::string _extension = input_file.extension().generic_string();
    if (_extension == ".ttf") {
        // compile_binary_or_json(import_font(input_file), output_file);
    } else if (_extension == ".glb" || _extension == ".gltf") {
        compile_ozz_resources(input_file, output_file.parent_path());
        compile_binary_or_json(import_mesh(input_file), output_file);
        compile_binary_or_json(import_armature(input_file), output_file.parent_path() / (output_file.stem().string() + "_armature.bin"));
    } else if (_extension == ".glsl") {
        compile_binary_or_json(import_shader(input_file), output_file);    
    } else if (_extension == ".wav") {
        // compile_binary_or_json(import_sound(input_file), output_file); 
    } else if (_extension == ".jpg") {
        compile_binary_or_json(import_texture(input_file), output_file); 
    } else {
        std::cerr << "Invalid input file with extension " << _extension << std::endl;
        std::terminate();
    }
}

int main(int argc, char* argv[])
{
    commands_map _commands = extract_args(argc, argv);
    if (process_help_command(_commands)) return 0;
    std::filesystem::path _input_dir = process_input_command(_commands);
    std::filesystem::path _output_dir = process_output_command(_commands);
    iterate_recursive(_input_dir, [&] (const std::filesystem::path& _input_file) {
        std::filesystem::path _relative_input_file = substract_relative(_input_dir, _input_file);
        std::filesystem::path _relative_output_file = _relative_input_file;
        _relative_output_file.replace_extension(".bin");
        std::filesystem::path _output_file = _output_dir / _relative_output_file;
        compile_resource(_input_file, _output_file);
    });
    return 0;
}
