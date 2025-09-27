#pragma once

#if defined(_MSC_VER) && _MSC_VER <= 1929
#define LUCARIA_DELETE_DEFAULT_SEMANTICS(object) \
    inline object() noexcept { }
#else
#define LUCARIA_DELETE_DEFAULT_SEMANTICS(object) \
    object() = delete;
#endif

