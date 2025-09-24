#include <iostream>
#include <string>

#include "etcpak.hpp"

void execute_etcpak(const etcpak_mode mode, const std::filesystem::path& etcpak_exe, const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    std::string _command = etcpak_exe.string().append(" ");
    if (mode == etcpak_mode::s3tc) { // no --etc2 (its ETC2 by default)
        _command.append("--dxtc ");
    }
    _command.append("--disable-heuristics "); // better quality
    _command.append(input_path.string()).append(" ");
    _command.append(output_path.string());
    // std::cout << _command << std::endl;
    const int _result = std::system(_command.c_str());
    if (_result != 0) {
        std::cout << "Tool etcpak failed with exit code " << _result << "." << std::endl;
        std::terminate();
    }
    std::cout << "   Exporting etc/s3tc compressed image " << output_path.filename() << std::endl;
}