#pragma once

#if !defined(__EMSCRIPTEN__) && !defined(AL_LIBTYPE_STATIC)
#define AL_LIBTYPE_STATIC 1
#endif

#include <AL/al.h>
#include <AL/alc.h>
