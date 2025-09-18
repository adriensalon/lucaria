cmake_minimum_required(VERSION 3.15)
project(lucaria_external_etcpak LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(etcpack_sources
    "upstream/Application.cpp"
    "upstream/Bitmap.cpp"
    "upstream/BitmapDownsampled.cpp"
    "upstream/BlockData.cpp"
    "upstream/ColorSpace.cpp"
    "upstream/DataProvider.cpp"
    "upstream/Debug.cpp"
    "upstream/Dither.cpp"
    "upstream/Error.cpp"
    "upstream/getopt/getopt.c"
    "upstream/libpng/arm_init.c"
    "upstream/libpng/filter_neon_intrinsics.c"
    "upstream/libpng/filter_sse2_intrinsics.c"
    "upstream/libpng/intel_init.c"
    "upstream/libpng/palette_neon_intrinsics.c"
    "upstream/libpng/png.c"
    "upstream/libpng/pngerror.c"
    "upstream/libpng/pngget.c"
    "upstream/libpng/pngmem.c"
    "upstream/libpng/pngpread.c"
    "upstream/libpng/pngread.c"
    "upstream/libpng/pngrio.c"
    "upstream/libpng/pngrtran.c"
    "upstream/libpng/pngrutil.c"
    "upstream/libpng/pngset.c"
    "upstream/libpng/pngtrans.c"
    "upstream/libpng/pngwio.c"
    "upstream/libpng/pngwrite.c"
    "upstream/libpng/pngwtran.c"
    "upstream/libpng/pngwutil.c"
    "upstream/lz4/lz4.c"
    "upstream/mmap.cpp"
    "upstream/ProcessDxtc.cpp"
    "upstream/ProcessRGB.cpp"
    "upstream/System.cpp"
    "upstream/Tables.cpp"
    "upstream/TaskDispatch.cpp"
    "upstream/Timing.cpp"
    # "upstream/tracy/TracyClient.cpp"
    "upstream/zlib/adler32.c"
    "upstream/zlib/compress.c"
    "upstream/zlib/crc32.c"
    "upstream/zlib/deflate.c"
    "upstream/zlib/gzlib.c"
    "upstream/zlib/infback.c"
    "upstream/zlib/inffas8664.c"
    "upstream/zlib/inffast.c"
    "upstream/zlib/inflate.c"
    "upstream/zlib/inftrees.c"
    "upstream/zlib/trees.c"
    "upstream/zlib/uncompr.c"
    "upstream/zlib/zutil.c"
)

add_executable(lucaria_external_etcpak ${etcpack_sources})
target_include_directories(lucaria_external_etcpak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/upstream")
target_include_directories(lucaria_external_etcpak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/upstream/zlib")
target_include_directories(lucaria_external_etcpak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/upstream/libpng")
target_include_directories(lucaria_external_etcpak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/upstream/lz4")
target_include_directories(lucaria_external_etcpak PRIVATE "${CMAKE_CURRENT_LIST_DIR}/upstream/getopt")
target_compile_definitions(lucaria_external_etcpak PRIVATE NO_GZIP)
target_compile_definitions(lucaria_external_etcpak PRIVATE PNG_INTEL_SSE)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lucaria_external_etcpak PRIVATE NDEBUG)
endif()

if(MSVC)
    target_compile_definitions(lucaria_external_etcpak PRIVATE NOMINMAX)
    target_compile_definitions(lucaria_external_etcpak PRIVATE __SSE4_1__)
    target_compile_definitions(lucaria_external_etcpak PRIVATE __AVX2__)
    target_compile_options(lucaria_external_etcpak PRIVATE /arch:AVX2)
    target_compile_options(lucaria_external_etcpak PRIVATE /GL)
    target_link_options(lucaria_external_etcpak PRIVATE /LTCG:incremental)
    list(APPEND etcpack_sources "upstream/getopt/getopt.c")
else()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        add_compile_options(-mcpu=native)
    else()
        add_compile_options(-march=native)
    endif()
endif()

if (MSVC)
    target_compile_options(lucaria_external_etcpak PRIVATE /wd4018)
    target_compile_options(lucaria_external_etcpak PRIVATE /wd4244)
    target_compile_options(lucaria_external_etcpak PRIVATE /wd4996)
    target_compile_options(lucaria_external_etcpak PRIVATE /wd4267)
    target_compile_options(lucaria_external_etcpak PRIVATE /wd4715)
endif()
