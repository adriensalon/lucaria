#pragma once

#include <string>
#include <string_view>

#if defined(LUCARIA_DEBUG)
#define LUCARIA_DEBUG_ERROR(Message) lucaria::detail::runtime_error(__FILE__, __LINE__, Message);
#define LUCARIA_DEBUG_ASSERT(Condition, Message) \
    if (!(Condition)) {                          \
        LUCARIA_DEBUG_ERROR(Message);            \
    }
#define LUCARIA_DEBUG_OPENAL_ASSERT lucaria::detail::runtime_openal_assert(__FILE__, __LINE__);
#define LUCARIA_DEBUG_OPENGL_ASSERT lucaria::detail::runtime_opengl_assert(__FILE__, __LINE__);
#else
#define LUCARIA_DEBUG_ERROR(Message)
#define LUCARIA_DEBUG_ASSERT(Condition, Message)
#define LUCARIA_DEBUG_OPENAL_ASSERT
#define LUCARIA_DEBUG_OPENGL_ASSERT
#endif

namespace lucaria {
namespace detail {

    void runtime_error(std::string_view file, const int line, const std::string& message);
    void runtime_openal_assert(std::string_view file, const int line);
    void runtime_opengl_assert(std::string_view file, const int line);

}
}
