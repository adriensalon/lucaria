#pragma once

#if defined(LUCARIA_BACKEND_OPENGL)
#if defined(LUCARIA_PLATFORM_ANDROID)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#endif

#if defined(LUCARIA_PLATFORM_WEB)
#include <GLES3/gl3.h>
#endif

#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
#endif

#if defined(LUCARIA_BACKEND_VULKAN)
#if defined(LUCARIA_PLATFORM_WIN32) || defined(LUCARIA_PLATFORM_LINUX)
#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#if defined(LUCARIA_PLATFORM_PSP)
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#endif
#endif
