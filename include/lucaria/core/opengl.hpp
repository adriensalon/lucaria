#pragma once

#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#elif defined(__ANDROID__)
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
