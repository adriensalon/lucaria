#include <iostream>
#include <string>

#include "oggenc.hpp"

void execute_oggenc(const std::filesystem::path& executable_path, const std::filesystem::path& input_path, const std::filesystem::path& output_directory)
{
    const std::filesystem::path _destination_file = (output_directory / input_path.stem()).string() + ".bin";
    std::string _command = executable_path.string() + " "  + input_path.string() + " -o " + _destination_file.string() + " --downmix";
#if _WIN32
    _command += " > NUL 2>&1";
#elif __LINUX__
    _command += " > /dev/null 2>&1";
#endif
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Tool oggenc failed with exit code " << _result << "." << std::endl;
        std::terminate();
    }
    std::cout << "   Exporting ogg compressed audio " << _destination_file.filename() << std::endl;
}
