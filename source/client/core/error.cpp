#include <exception>
#include <iostream>
#include <string>

#include <lucaria/core/error.hpp>

namespace lucaria {

void runtime_error(std::string_view file, const int line, const std::string& message)
{
    std::string _text;
    _text.append("Lucaria error in '").append(file.data()).append("' ");
    _text.append("line '").append(std::to_string(line)).append(" ");
    _text.append("with message: ").append(message);
    std::cout << _text << std::endl;
#if defined(__EMSCRIPTEN__)
    std::terminate();
#else
    throw std::runtime_error(_text);
#endif
}

}