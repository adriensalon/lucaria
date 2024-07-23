#include <iostream>
#include <string>

#include <tool/oggenc.hpp>

void execute_oggenc(const std::filesystem::path& input_path, const std::filesystem::path& output_directory)
{
    const std::filesystem::path _oggenc_file = std::filesystem::current_path() / "compiler" / "oggenc";
    const std::filesystem::path _destination_file = (output_directory / input_path.stem()).string() + ".bin";
    const std::string _command = _oggenc_file.string() + " "  + input_path.string() + " -o " + _destination_file.string();
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Tool oggenc failed with exit code " << _result << "." << std::endl;
        std::terminate();
    }
    std::cout << "   Exporting ogg compressed audio " << _destination_file.filename() << std::endl;
}
