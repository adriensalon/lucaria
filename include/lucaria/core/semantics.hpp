#pragma once

// workaround for old msvc that touches default ctor from its std17 std::future
#if defined(_MSC_VER) && _MSC_VER <= 1929
#define LUCARIA_DELETE_DEFAULT_SEMANTICS(object) \
    inline object() noexcept { }
#else
#define LUCARIA_DELETE_DEFAULT_SEMANTICS(object) \
    object() = delete;
#endif

