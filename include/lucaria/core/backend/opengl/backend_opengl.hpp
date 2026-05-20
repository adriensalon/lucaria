#pragma once

#if defined(LUCARIA_PLATFORM_ANDROID)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif

#if defined(LUCARIA_PLATFORM_WEB)
#include <GLES3/gl3.h>
#endif

#if defined(LUCARIA_PLATFORM_GLFW)
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
