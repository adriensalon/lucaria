#pragma once

// arch detection
#if INTPTR_MAX == INT32_MAX
#define LUCARIA_ARCH_32 1
#define LUCARIA_ARCH_64 0
#elif INTPTR_MAX == INT64_MAX
#define LUCARIA_ARCH_32 0
#define LUCARIA_ARCH_64 1
#else
#error Unknown pointer size or missing size macros. Please define INTPTR_MAX && (INT32_MAX || INT64_MAX).
#endif

// cpu detection
#if defined(__EMSCRIPTEN__)
#define LUCARIA_CPU_X86 0
#define LUCARIA_CPU_ARM 0
#define LUCARIA_CPU_WASM 1
#elif defined(__x86_64__) || defined(_M_X64) || defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86) // x64
#define LUCARIA_CPU_X86 1
#define LUCARIA_CPU_ARM 0
#define LUCARIA_CPU_WASM 0
#elif defined(__arm__) || defined(__aarch64__) || defined(_ARM) || defined(_M_ARM) || defined(__arm)
#define LUCARIA_CPU_X86 0
#define LUCARIA_CPU_ARM 1
#define LUCARIA_CPU_WASM 0
#else
#error "Undefined CPU"
#endif

// platform detection
#if defined(__ANDROID__)
#define LUCARIA_PLATFORM_NAME "android"
#define LUCARIA_PLATFORM_ANDROID 1
#define LUCARIA_PLATFORM_WEB 0
#define LUCARIA_PLATFORM_WIN32 0
#elif defined(__EMSCRIPTEN__)
#define LUCARIA_PLATFORM_NAME "web"
#define LUCARIA_PLATFORM_ANDROID 0
#define LUCARIA_PLATFORM_WEB 1
#define LUCARIA_PLATFORM_WIN32 0
#elif defined(_WIN32)
#define LUCARIA_PLATFORM_NAME "win32"
#define LUCARIA_PLATFORM_ANDROID 0
#define LUCARIA_PLATFORM_WEB 0
#define LUCARIA_PLATFORM_WIN32 1
#else
#error "Unsupported platform"
#endif

// compiler detection
#if defined(_MSC_VER)
#define LUCARIA_COMPILER_MSVC 1
#define LUCARIA_COMPILER_CLANG 0
#define LUCARIA_COMPILER_GCC 0
#elif defined(__clang__)
#define LUCARIA_COMPILER_MSVC 0
#define LUCARIA_COMPILER_CLANG 1
#define LUCARIA_COMPILER_GCC 0
#elif defined(__gcc__)
#define LUCARIA_COMPILER_MSVC 0
#define LUCARIA_COMPILER_CLANG 0
#define LUCARIA_COMPILER_GCC 1
#else
#error "Unsupported compiler"
#endif

// config detection
// #if defined(NDEBUG)
// #define LUCARIA_CONFIG_DEBUG 0
// #define LUCARIA_CONFIG_RELEASE 1
// #else
// #define LUCARIA_CONFIG_DEBUG 1
// #define LUCARIA_CONFIG_RELEASE 0
// #endif

// inline for LTO
#if LUCARIA_COMPILER_CLANG || LUCARIA_COMPILER_GCC
#define LUCARIA_INLINE inline __attribute__((always_inline))
#elif LUCARIA_COMPILER_MSVC
#define LUCARIA_INLINE __forceinline
#endif

// options
#if !defined(LUCARIA_OPTION_PACKAGE)
#error "Undefined Lucaria option LUCARIA_OPTION_PACKAGE"
#endif

// workaround for old MSVC that touches default ctor from its std17 std::future
#if LUCARIA_COMPILER_MSVC && _MSC_VER <= 1929
#define LUCARIA_DELETE_DEFAULT(object) \
    inline object() noexcept { }
#else
#define LUCARIA_DELETE_DEFAULT(object) \
    object() = delete;
#endif

