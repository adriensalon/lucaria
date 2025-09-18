#include <iostream>
#include <string>

#include "woff2compress.hpp"

void execute_woff2compress(const std::filesystem::path& woff2compress_exe, const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    std::string _command = woff2compress_exe.string() + " " + input_path.string();
#if _WIN32
    _command += " > NUL 2>&1";
#elif __LINUX__
    _command += " > /dev/null 2>&1";
#endif
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Tool woff2compress failed with exit code " << _result << "." << std::endl;
        std::terminate();
    }
    const std::filesystem::path _generated_path = input_path.parent_path() / (input_path.stem().string() + ".woff2");
    if (std::filesystem::exists(_generated_path)) {
        std::filesystem::rename(_generated_path, output_path);
    }
    std::cout << "   Exporting woff2 compressed font " << output_path.filename() << std::endl;
}