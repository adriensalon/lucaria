#pragma once

// workaround for old MSVC that touches default ctor from its std17 std::future
#if defined(_MSC_VER) && _MSC_VER <= 1929
#define LUCARIA_DELETE_DEFAULT(Object) \
    inline Object() noexcept { }
#else
#define LUCARIA_DELETE_DEFAULT(Object) \
    Object() = delete;
#endif
