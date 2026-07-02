#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#if defined(LUCARIA_BACKEND_OPENGL)
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
#define GLAD_VULKAN_IMPLEMENTATION
#include <glad/vulkan.h>
#endif
#endif
