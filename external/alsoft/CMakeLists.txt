# if(EMSCRIPTEN)
#     return()
# endif()



# configure_file(
#     "upstream/alsoft/version.h.in"
#     "upstream/alsoft/version.h")

# configure_file(
#     "upstream/alsoft/config.h.in"
#     "upstream/alsoft/config.h")

# configure_file(
#     "upstream/alsoft/config_backends.h.in"
#     "upstream/alsoft/config_backends.h")

# configure_file(
#     "upstream/alsoft/config_simd.h.in"
#     "upstream/alsoft/config_simd.h")

# set(alsoft_common_sources
#     "upstream/alsoft/common/alassert.cpp"
#     "upstream/alsoft/common/alcomplex.cpp"
#     "upstream/alsoft/common/alsem.cpp"
#     "upstream/alsoft/common/alstring.cpp"
#     "upstream/alsoft/common/althrd_setname.cpp"
#     "upstream/alsoft/common/dynload.cpp"
#     "upstream/alsoft/common/pffft.cpp"
#     "upstream/alsoft/common/polyphase_resampler.cpp"
#     "upstream/alsoft/common/ringbuffer.cpp"
#     "upstream/alsoft/common/strutils.cpp")

# set(alsoft_core_sources
#     "upstream/alsoft/core/ambdec.cpp"
#     "upstream/alsoft/core/ambidefs.cpp"
#     "upstream/alsoft/core/bformatdec.cpp"
#     "upstream/alsoft/core/bs2b.cpp"
#     "upstream/alsoft/core/bsinc_tables.cpp"
#     "upstream/alsoft/core/buffer_storage.cpp"
#     "upstream/alsoft/core/context.cpp"
#     "upstream/alsoft/core/converter.cpp"
#     "upstream/alsoft/core/cpu_caps.cpp"
#     "upstream/alsoft/core/cubic_tables.cpp"
#     "upstream/alsoft/core/devformat.cpp"
#     "upstream/alsoft/core/device.cpp"
#     "upstream/alsoft/core/effectslot.cpp"
#     "upstream/alsoft/core/filters/biquad.cpp"
#     "upstream/alsoft/core/filters/nfc.cpp"
#     "upstream/alsoft/core/filters/splitter.cpp"
#     "upstream/alsoft/core/fmt_traits.cpp"
#     "upstream/alsoft/core/fpu_ctrl.cpp"
#     "upstream/alsoft/core/helpers.cpp"
#     "upstream/alsoft/core/hrtf.cpp"
#     "upstream/alsoft/core/logging.cpp"
#     "upstream/alsoft/core/mastering.cpp"
#     "upstream/alsoft/core/mixer.cpp"
#     "upstream/alsoft/core/storage_formats.cpp"
#     "upstream/alsoft/core/uhjfilter.cpp"
#     "upstream/alsoft/core/uiddefs.cpp"
#     "upstream/alsoft/core/voice.cpp")

# set(alsoft_al_sources
#     "upstream/alsoft/al/auxeffectslot.cpp"
#     "upstream/alsoft/al/buffer.cpp"
#     "upstream/alsoft/al/debug.cpp"
#     "upstream/alsoft/al/effect.cpp"
#     "upstream/alsoft/al/effects/autowah.cpp"
#     "upstream/alsoft/al/effects/chorus.cpp"
#     "upstream/alsoft/al/effects/compressor.cpp"
#     "upstream/alsoft/al/effects/convolution.cpp"
#     "upstream/alsoft/al/effects/dedicated.cpp"
#     "upstream/alsoft/al/effects/distortion.cpp"
#     "upstream/alsoft/al/effects/echo.cpp"
#     "upstream/alsoft/al/effects/effects.cpp"
#     "upstream/alsoft/al/effects/equalizer.cpp"
#     "upstream/alsoft/al/effects/fshifter.cpp"
#     "upstream/alsoft/al/effects/modulator.cpp"
#     "upstream/alsoft/al/effects/null.cpp"
#     "upstream/alsoft/al/effects/pshifter.cpp"
#     "upstream/alsoft/al/effects/reverb.cpp"
#     "upstream/alsoft/al/effects/vmorpher.cpp"
#     "upstream/alsoft/al/error.cpp"
#     "upstream/alsoft/al/event.cpp"
#     "upstream/alsoft/al/extension.cpp"
#     "upstream/alsoft/al/filter.cpp"
#     "upstream/alsoft/al/listener.cpp"
#     "upstream/alsoft/al/source.cpp"
#     "upstream/alsoft/al/state.cpp")

# set(alsoft_alc_sources
#     "upstream/alsoft/alc/alc.cpp"
#     "upstream/alsoft/alc/alu.cpp"
#     "upstream/alsoft/alc/alconfig.cpp"
#     "upstream/alsoft/alc/context.cpp"
#     "upstream/alsoft/alc/device.cpp"
#     "upstream/alsoft/alc/effects/autowah.cpp"
#     "upstream/alsoft/alc/effects/chorus.cpp"
#     "upstream/alsoft/alc/effects/compressor.cpp"
#     "upstream/alsoft/alc/effects/convolution.cpp"
#     "upstream/alsoft/alc/effects/dedicated.cpp"
#     "upstream/alsoft/alc/effects/distortion.cpp"
#     "upstream/alsoft/alc/effects/echo.cpp"
#     "upstream/alsoft/alc/effects/equalizer.cpp"
#     "upstream/alsoft/alc/effects/fshifter.cpp"
#     "upstream/alsoft/alc/effects/modulator.cpp"
#     "upstream/alsoft/alc/effects/null.cpp"
#     "upstream/alsoft/alc/effects/pshifter.cpp"
#     "upstream/alsoft/alc/effects/reverb.cpp"
#     "upstream/alsoft/alc/effects/vmorpher.cpp"
#     "upstream/alsoft/alc/events.cpp"
#     "upstream/alsoft/alc/panning.cpp")

# set(alsoft_sources
#     ${alsoft_common_sources}
#     ${alsoft_core_sources}
#     ${alsoft_al_sources}
#     ${alsoft_alc_sources})

# add_library(lucaria_external_alsoft STATIC EXCLUDE_FROM_ALL ${alsoft_sources})
# set_target_properties(lucaria_external_alsoft PROPERTIES CXX_STANDARD 17)
# set_target_properties(lucaria_external_alsoft PROPERTIES C_STANDARD 11)
# target_compile_definitions(lucaria_external_alsoft PRIVATE AL_LIBTYPE_STATIC)
# target_include_directories(lucaria_external_alsoft PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/upstream/alsoft")
# target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft")
# target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/common")
# target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/core")
# target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/al")
# target_include_directories(lucaria_external_alsoft PRIVATE "upstream/alsoft/alc")
# target_include_directories(lucaria_external_alsoft PUBLIC "upstream/alsoft/include")

# if(WIN32)
#     target_compile_definitions(lucaria_external_alsoft PRIVATE NOMINMAX)
# endif()

# if(MSVC)
#     target_compile_options(lucaria_external_alsoft PRIVATE /wd4018)
#     target_compile_options(lucaria_external_alsoft PRIVATE /wd4065)
#     target_compile_options(lucaria_external_alsoft PRIVATE /wd4996)
#     target_compile_options(lucaria_external_alsoft PRIVATE /wd5030)
# endif()

add_subdirectory(upstream/alsoft)