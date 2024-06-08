#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <core/mesh.hpp>
#include <core/shader.hpp>
#include <core/texture.hpp>

namespace lucaria {

/// @brief 
using commands_map = std::unordered_map<std::string, std::vector<std::string>>;

/// @brief 
/// @param argc 
/// @param argv 
/// @return 
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
/// @param input_path 
/// @param output_path 
void compile_resource(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    // // get output file name and path
    // std::string _name;
    // _name = json_get_string(json_root, "name", []() {
    //     console::log("error : ", console::text_color::red);
    //     exceptions::throw_error();
    // });
    // std::filesystem::path _output_file_path = output_path / (_name + ".bin");
    // file::create_directory_if_needed(_output_file_path.parent_path());

    // // get resource type and dispatch
    // std::string _type;
    // _type = json_get_string(json_root, "type", []() {
    //     console::log("error : ", console::text_color::red);
    //     exceptions::throw_error();
    // });
    // if (_type == "sound")
    //     audio::compile_sound(json_root, _output_file_path);
    // else if (_type == "mesh")
    //     graphics::compile_mesh(json_root, _output_file_path);
    // else if (_type == "texture")
    //     graphics::compile_texture(json_root, _output_file_path);
    // else if (_type == "cubemap")
    //     graphics::compile_cubemap(json_root, _output_file_path);
    // else if (_type == "shader")
    //     graphics::compile_shader(json_root, _output_file_path);
    // else {
    //     console::log("error : invalid resource type '" + _type + "' is not 'audio' | 'mesh' | 'texture' | 'cubemap' | 'shader'", console::text_color::red);
    //     exceptions::throw_error();
    // }

    // std::filesystem::path _resolved_output_file_path = std::filesystem::current_path() / _output_file_path;
    // console::log("message : asset type '" + _type + "' compiled at '" + _resolved_output_file_path.generic_string() + "'", console::text_color::green);
}

}

int main(int argc, char* argv[])
{
    lucaria::commands_map _commands = lucaria::extract_args(argc, argv);
    if (lucaria::process_help_command(_commands)) return 0;
    std::filesystem::path _input_dir = lucaria::process_input_command(_commands);
    std::filesystem::path _output_dir = lucaria::process_output_command(_commands);
    return 0;
}
