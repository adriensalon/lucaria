#pragma once

#include <string>
#include <string_view>

#include <lucaria/core/platform.hpp>

#if defined(LUCARIA_CONFIG_DEBUG)
#define LUCARIA_RUNTIME_ERROR(message) lucaria::runtime_error(__FILE__, __LINE__, message);
#define LUCARIA_RUNTIME_OPENAL_ASSERT lucaria::runtime_openal_assert(__FILE__, __LINE__);
#define LUCARIA_RUNTIME_OPENGL_ASSERT lucaria::runtime_opengl_assert(__FILE__, __LINE__);
#else
#define LUCARIA_RUNTIME_ERROR(message)
#define LUCARIA_RUNTIME_OPENAL_ASSERT
#define LUCARIA_RUNTIME_OPENGL_ASSERT
#endif

namespace lucaria {

/// @brief Throws an error on platforms that support it, logs and terminates otherwise
/// @param file the file where the error happens
/// @param line the line where the error happens
/// @param message the error message
void runtime_error(std::string_view file, const int line, const std::string& message);

/// @brief Throws any pending OpenAL errors
/// @param file the file where the assert happens
/// @param line the line where the assert happens
void runtime_openal_assert(std::string_view file, const int line);

/// @brief Throws any pending OpenGL errors
/// @param file the file where the assert happens
/// @param line the line where the assert happens
void runtime_opengl_assert(std::string_view file, const int line);

}
