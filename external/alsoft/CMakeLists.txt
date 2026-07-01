if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    # Fix compile failure with armv7 deployment target >= 11.0, xcode clang
    # will report:
    # error: invalid iOS deployment version '--target=armv7-apple-ios13.6',
    # iOS 10 is the maximum deployment target for 32-bit targets
    # If CMAKE_OSX_DEPLOYMENT_TARGET is not defined, cmake will choose latest
    # deployment target
    if("${CMAKE_OSX_ARCHITECTURES}" MATCHES ".*armv7.*")
        if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET
            OR NOT CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "11.0")
            message(STATUS "Forcing iOS deployment target to 10.0 for armv7")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.0" CACHE STRING "Minimum OS X deployment version" FORCE)
        endif()
    endif()
endif()

# fmt
add_library(lucaria_external_fmt STATIC "upstream/alsoft/fmt-11.0.2/src/format.cc" "upstream/alsoft/fmt-11.0.2/src/os.cc")
target_include_directories(lucaria_external_fmt PUBLIC "upstream/alsoft/fmt-11.0.2/include")
set_target_properties(lucaria_external_fmt PROPERTIES CXX_STANDARD 11)
if(MSVC)
  	target_compile_options(lucaria_external_fmt PUBLIC $<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
endif()

# openal
if(PSP)
    set(alsoft_sources 
		"upstream/alsoft_psp/src/OpenAL/alAuxEffectSlot.c"
		"upstream/alsoft_psp/src/OpenAL/alBuffer.c"
		"upstream/alsoft_psp/src/OpenAL/alEffect.c"
		"upstream/alsoft_psp/src/OpenAL/alError.c"
		"upstream/alsoft_psp/src/OpenAL/alExtension.c"
		"upstream/alsoft_psp/src/OpenAL/alFilter.c"
		"upstream/alsoft_psp/src/OpenAL/alListener.c"
		"upstream/alsoft_psp/src/OpenAL/alSource.c"
		"upstream/alsoft_psp/src/OpenAL/alState.c"
		"upstream/alsoft_psp/src/OpenAL/alThunk.c"
		"upstream/alsoft_psp/src/Alc/ALc.c"
		"upstream/alsoft_psp/src/Alc/ALu.c"
		"upstream/alsoft_psp/src/Alc/alcConfig.c"
		"upstream/alsoft_psp/src/Alc/alcReverb.c"
		"upstream/alsoft_psp/src/Alc/alcRing.c"
		"upstream/alsoft_psp/src/Alc/alcThread.c"
		"upstream/alsoft_psp/src/Alc/bs2b.c"
		"upstream/alsoft_psp/src/Alc/psp.c"
		"upstream/alsoft_psp/src/Alc/wave.c")
else()
	set(alsoft_sources
		"upstream/alsoft/common/alassert.cpp"
		"upstream/alsoft/common/alcomplex.cpp"
		"upstream/alsoft/common/alsem.cpp"
		"upstream/alsoft/common/alstring.cpp"
		"upstream/alsoft/common/althrd_setname.cpp"
		"upstream/alsoft/common/dynload.cpp"
		"upstream/alsoft/common/pffft.cpp"
		"upstream/alsoft/common/polyphase_resampler.cpp"
		"upstream/alsoft/common/ringbuffer.cpp"
		"upstream/alsoft/common/strutils.cpp"
		"upstream/alsoft/core/ambdec.cpp"
		"upstream/alsoft/core/ambidefs.cpp"
		"upstream/alsoft/core/bformatdec.cpp"
		"upstream/alsoft/core/bs2b.cpp"
		"upstream/alsoft/core/bsinc_tables.cpp"
		"upstream/alsoft/core/buffer_storage.cpp"
		"upstream/alsoft/core/context.cpp"
		"upstream/alsoft/core/converter.cpp"
		"upstream/alsoft/core/cpu_caps.cpp"
		"upstream/alsoft/core/cubic_tables.cpp"
		"upstream/alsoft/core/devformat.cpp"
		"upstream/alsoft/core/device.cpp"
		"upstream/alsoft/core/effectslot.cpp"
		"upstream/alsoft/core/filters/biquad.cpp"
		"upstream/alsoft/core/filters/nfc.cpp"
		"upstream/alsoft/core/filters/splitter.cpp"
		"upstream/alsoft/core/fmt_traits.cpp"
		"upstream/alsoft/core/fpu_ctrl.cpp"
		"upstream/alsoft/core/helpers.cpp"
		"upstream/alsoft/core/hrtf.cpp"
		"upstream/alsoft/core/logging.cpp"
		"upstream/alsoft/core/mastering.cpp"
		"upstream/alsoft/core/mixer.cpp"
		"upstream/alsoft/core/storage_formats.cpp"
		"upstream/alsoft/core/uhjfilter.cpp"
		"upstream/alsoft/core/uiddefs.cpp"
		"upstream/alsoft/core/voice.cpp"
		"upstream/alsoft/core/except.cpp"
		"upstream/alsoft/core/mixer/mixer_c.cpp"
		"upstream/alsoft/al/auxeffectslot.cpp"
		"upstream/alsoft/al/buffer.cpp"
		"upstream/alsoft/al/debug.cpp"
		"upstream/alsoft/al/effect.cpp"
		"upstream/alsoft/al/effects/autowah.cpp"
		"upstream/alsoft/al/effects/chorus.cpp"
		"upstream/alsoft/al/effects/compressor.cpp"
		"upstream/alsoft/al/effects/convolution.cpp"
		"upstream/alsoft/al/effects/dedicated.cpp"
		"upstream/alsoft/al/effects/distortion.cpp"
		"upstream/alsoft/al/effects/echo.cpp"
		"upstream/alsoft/al/effects/effects.cpp"
		"upstream/alsoft/al/effects/equalizer.cpp"
		"upstream/alsoft/al/effects/fshifter.cpp"
		"upstream/alsoft/al/effects/modulator.cpp"
		"upstream/alsoft/al/effects/null.cpp"
		"upstream/alsoft/al/effects/pshifter.cpp"
		"upstream/alsoft/al/effects/reverb.cpp"
		"upstream/alsoft/al/effects/vmorpher.cpp"
		"upstream/alsoft/al/error.cpp"
		"upstream/alsoft/al/event.cpp"
		"upstream/alsoft/al/extension.cpp"
		"upstream/alsoft/al/filter.cpp"
		"upstream/alsoft/al/listener.cpp"
		"upstream/alsoft/al/source.cpp"
		"upstream/alsoft/al/state.cpp"
		"upstream/alsoft/alc/alc.cpp"
		"upstream/alsoft/alc/alu.cpp"
		"upstream/alsoft/alc/alconfig.cpp"
		"upstream/alsoft/alc/context.cpp"
		"upstream/alsoft/alc/device.cpp"
		"upstream/alsoft/alc/effects/autowah.cpp"
		"upstream/alsoft/alc/effects/chorus.cpp"
		"upstream/alsoft/alc/effects/compressor.cpp"
		"upstream/alsoft/alc/effects/convolution.cpp"
		"upstream/alsoft/alc/effects/dedicated.cpp"
		"upstream/alsoft/alc/effects/distortion.cpp"
		"upstream/alsoft/alc/effects/echo.cpp"
		"upstream/alsoft/alc/effects/equalizer.cpp"
		"upstream/alsoft/alc/effects/fshifter.cpp"
		"upstream/alsoft/alc/effects/modulator.cpp"
		"upstream/alsoft/alc/effects/null.cpp"
		"upstream/alsoft/alc/effects/pshifter.cpp"
		"upstream/alsoft/alc/effects/reverb.cpp"
		"upstream/alsoft/alc/effects/vmorpher.cpp"
		"upstream/alsoft/alc/events.cpp"
		"upstream/alsoft/alc/panning.cpp"
		"upstream/alsoft/alc/backends/base.cpp"
		"upstream/alsoft/alc/backends/loopback.cpp"
		"upstream/alsoft/alc/backends/null.cpp")
endif()

set(LIB_MAJOR_VERSION "1")
set(LIB_MINOR_VERSION "24")
set(LIB_REVISION "1")
set(LIB_VERSION "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_REVISION}")
set(LIB_VERSION_NUM ${LIB_MAJOR_VERSION},${LIB_MINOR_VERSION},${LIB_REVISION},0)

set(HAVE_STDC_FORMAT_MACROS 1)
set(HAVE_LIBATOMIC 0)
set(HAVE_LIBM 0)
set(HAVE_LIBDL 0)
set(HAVE_CPUID_H 0)
set(HAVE_INTRIN_H 0)
set(HAVE_GUIDDEF_H 0)
set(HAVE_XMMINTRIN_H 0)
set(HAVE_EMMINTRIN_H 0)
set(HAVE_PMMINTRIN_H 0)
set(HAVE_SMMINTRIN_H 0)
set(HAVE_ARM_NEON_H 0)
set(HAVE_PROC_PIDPATH 0)
set(HAVE_PTHREAD_H 0)
set(HAVE_PTHREAD_SETSCHEDPARAM 0)
set(HAVE_PTHREAD_SETNAME_NP 0)
set(HAVE_PTHREAD_SET_NAME_NP 0)
set(HAVE_PTHREAD_NP_H 0)
set(HAVE_GCC_GET_CPUID 0)
set(HAVE_CPUID_INTRINSIC 0)
set(HAVE_SSE_INTRINSICS 0)
set(HAVE_ALSA 0)
set(HAVE_OSS 0)
set(HAVE_PIPEWIRE 0)
set(HAVE_SOLARIS 0)
set(HAVE_SNDIO 0)
set(HAVE_PORTAUDIO 0)
set(HAVE_PULSEAUDIO 0)
set(HAVE_COREAUDIO 0)
set(HAVE_OPENSL 0)
set(HAVE_OBOE 0)
set(HAVE_OTHERIO 0)
set(HAVE_WAVE 0)
set(HAVE_SDL2 0)
set(HAVE_DSOUND 0)
set(HAVE_WASAPI 0)
set(HAVE_WINMM 0)
set(HAVE_SSE 0)
set(HAVE_SSE2 0)
set(HAVE_SSE3 0)
set(HAVE_SSE4_1 0)
set(HAVE_NEON 0)
set(HAVE_RTKIT 0)
set(ALSOFT_EAX 0)
set(ALSOFT_UWP 0)
set(ALSOFT_EMBED_HRTF_DATA 0)
set(EXPORT_DECL "")
set(ALSOFT_FORCE_ALIGN "")
if(WIN32)
    set(HAVE_GUIDDEF_H 1)
    set(HAVE_WINMM 1)
    set(HAVE_WASAPI 1)
    set(HAVE_DSOUND 1)
    list(APPEND alsoft_sources
        "upstream/alsoft/alc/backends/winmm.cpp"
        "upstream/alsoft/alc/backends/wasapi.cpp"
        "upstream/alsoft/alc/backends/dsound.cpp")
elseif(PSP)
    set(HAVE_WAVE 1)
elseif(ANDROID)
    set(HAVE_OPENSL 1)
	set(HAVE_PTHREAD_H 1)
    list(APPEND alsoft_sources 
		"upstream/alsoft/alc/backends/opensl.cpp")
endif()

configure_file(
	"upstream/alsoft/version.h.in" 
	"${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft/version.h")
configure_file(
	"upstream/alsoft/config.h.in" 
	"${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft/config.h")
configure_file(
	"upstream/alsoft/config_backends.h.in" 
	"${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft/config_backends.h")
configure_file(
	"upstream/alsoft/config_simd.h.in" 
	"${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft/config_simd.h")

add_library(lucaria_external_alsoft STATIC EXCLUDE_FROM_ALL ${alsoft_sources})
target_link_libraries(lucaria_external_alsoft PRIVATE lucaria_external_fmt)

if(PSP)
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft_psp")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft_psp/src")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft_psp/src/OpenAL/include")
	target_include_directories(lucaria_external_alsoft PUBLIC "upstream/alsoft_psp/src/include")
else()
	set_target_properties(lucaria_external_alsoft PROPERTIES CXX_STANDARD 17)
	set_target_properties(lucaria_external_alsoft PROPERTIES CXX_STANDARD_REQUIRED TRUE)
	set_target_properties(lucaria_external_alsoft PROPERTIES C_STANDARD 11)
	target_include_directories(lucaria_external_alsoft PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/common")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/core")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/al")
	target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/alc")
	target_include_directories(lucaria_external_alsoft PUBLIC "upstream/alsoft/include")
endif()

if(WIN32)
    target_link_libraries(lucaria_external_alsoft PUBLIC winmm avrt ole32 uuid dsound dxguid)
elseif(ANDROID)
    target_link_libraries(lucaria_external_alsoft PRIVATE log OpenSLES)
endif()

target_compile_definitions(lucaria_external_alsoft PUBLIC AL_LIBTYPE_STATIC)
target_compile_definitions(lucaria_external_alsoft PRIVATE AL_BUILD_LIBRARY)

if(PSP)
	target_compile_options(lucaria_external_alsoft PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/psp/fix_alint64.h")
else()
	target_compile_definitions(lucaria_external_alsoft PRIVATE AL_ALEXT_PROTOTYPES
		"AL_API="
		"ALC_API=")
endif()

if(WIN32)
    target_compile_definitions(lucaria_external_alsoft PRIVATE _WIN32 NOMINMAX)
	if(MINGW)
    	target_compile_definitions(lucaria_external_alsoft PRIVATE __USE_MINGW_ANSI_STDIO)
	endif()
endif()

if(MSVC)
	target_compile_definitions(lucaria_external_alsoft PRIVATE
		_CRT_SECURE_NO_WARNINGS
		_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
    target_compile_options(lucaria_external_alsoft PRIVATE /permissive-)
    target_compile_options(lucaria_external_alsoft PRIVATE /wd4018)
    target_compile_options(lucaria_external_alsoft PRIVATE /wd4065)
    target_compile_options(lucaria_external_alsoft PRIVATE /wd4996)
    target_compile_options(lucaria_external_alsoft PRIVATE /wd5030)
endif()
