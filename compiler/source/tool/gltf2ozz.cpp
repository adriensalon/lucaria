#include <filesystem>
#include <iostream>
#include <string>

void execute_gltf2ozz(const std::filesystem::path& input_path, const std::filesystem::path& output_directory)
{
    const std::string _command = std::filesystem::current_path().string() + "/compiler/gltf2ozz --file=" + input_path.string();
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Error: gltf2ozz command failed with exit code " << _result << std::endl;
        std::terminate();
    }
    const std::filesystem::path _current_path = std::filesystem::current_path();
    for (const std::filesystem::directory_entry& _entry : std::filesystem::directory_iterator(_current_path)) {
        if (std::filesystem::is_regular_file(_entry.path()) && _entry.path().extension().string() == ".ozz") {
            std::filesystem::path _destination_file;
            if (_entry.path().filename().string() == "skeleton.ozz") {
                _destination_file = output_directory / (input_path.stem().string() + "_skeleton.bin");
                std::cout << "   Exporting skeleton data binary..." << std::endl;
            } else {
                _destination_file = output_directory / (input_path.stem().string() + "_animation_" + _entry.path().stem().string() + ".bin");
                std::cout << "   Exporting animation data binary..." << std::endl;
            }
            std::filesystem::rename(_entry.path(), _destination_file);
        }
    }
}