#pragma once

#if defined(LUCARIA_BACKEND_VULKAN)
#define GLFW_INCLUDE_VULKAN
#else
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#if defined(LUCARIA_BACKEND_OPENGL)
#include <glad/gl.h>
#endif
