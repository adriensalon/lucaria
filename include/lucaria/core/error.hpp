#pragma once

#include <string_view>

#if defined(LUCARIA_DEBUG)
#define LUCARIA_RUNTIME_ERROR(message) lucaria::runtime_error(__FILE__, __LINE__, message);
#else
#define LUCARIA_RUNTIME_ERROR(message)
#endif

namespace lucaria {

void runtime_error(std::string_view file, const int line, const std::string& message);

}