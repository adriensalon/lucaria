#pragma once

#include <string_view>

#if defined(LUCARIA_DEBUG)
#define LUCARIA_RUNTIME_ERROR(message) lucaria::runtime_error(__FILE__, __LINE__, message);
#define LUCARIA_RUNTIME_OPENAL_ASSERT lucaria::runtime_openal_assert(__FILE__, __LINE__);
#define LUCARIA_RUNTIME_OPENGL_ASSERT lucaria::runtime_opengl_assert(__FILE__, __LINE__);
#else
#define LUCARIA_RUNTIME_ERROR(message)
#define LUCARIA_RUNTIME_OPENAL_ASSERT
#define LUCARIA_RUNTIME_OPENGL_ASSERT
#endif

namespace lucaria {

void runtime_error(std::string_view file, const int line, const std::string& message);
void runtime_openal_assert(std::string_view file, const int line);
void runtime_opengl_assert(std::string_view file, const int line);

}