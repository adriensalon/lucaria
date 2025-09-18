
option(ASSIMP_DOUBLE_PRECISION "Set to ON to enable double precision processing" OFF)

set(GIT_COMMIT_HASH 0)
set(ASSIMP_VERSION_MAJOR 5)
set(ASSIMP_VERSION_MINOR 4)
set(ASSIMP_VERSION_PATCH 1)
set(ASSIMP_VERSION ${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH})
set(ASSIMP_SOVERSION 5)

configure_file(
  "upstream/assimp/include/assimp/revision.h.in"
  "${CMAKE_CURRENT_LIST_DIR}/include/assimp/revision.h")

configure_file(
  "upstream/assimp/include/assimp/config.h.in"
  "${CMAKE_CURRENT_LIST_DIR}/include/assimp/config.h")

file(GLOB_RECURSE assimp_asset_lib_fbx_sources "upstream/assimp/code/AssetLib/FBX/*.cpp")
file(GLOB_RECURSE assimp_asset_lib_gltf_sources "upstream/assimp/code/AssetLib/glTF/*.cpp")
file(GLOB_RECURSE assimp_asset_lib_gltf2_sources "upstream/assimp/code/AssetLib/glTF2/*.cpp")

set(assimp_asset_lib_sources
    ${assimp_asset_lib_fbx_sources}
    ${assimp_asset_lib_gltf_sources}
    ${assimp_asset_lib_gltf2_sources})

file(GLOB_RECURSE assimp_common_sources "upstream/assimp/code/Common/*.cpp")
file(GLOB_RECURSE assimp_geometry_sources "upstream/assimp/code/Geometry/*.cpp")
file(GLOB_RECURSE assimp_material_sources "upstream/assimp/code/Material/*.cpp")
file(GLOB_RECURSE assimp_pbrt_sources "upstream/assimp/code/Pbrt/*.cpp")
file(GLOB_RECURSE assimp_post_processing_sources "upstream/assimp/code/PostProcessing/*.cpp")

set(assimp_sources
    ${assimp_asset_lib_sources}
    ${assimp_common_sources}
    ${assimp_geometry_sources}
    ${assimp_material_sources}
    ${assimp_pbrt_sources}
    ${assimp_post_processing_sources})

set(assimp_zlib_sources
    "upstream/assimp/contrib/zlib/adler32.c"
    "upstream/assimp/contrib/zlib/compress.c"
    "upstream/assimp/contrib/zlib/crc32.c"
    "upstream/assimp/contrib/zlib/deflate.c"
    "upstream/assimp/contrib/zlib/gzclose.c"
    "upstream/assimp/contrib/zlib/gzlib.c"
    "upstream/assimp/contrib/zlib/gzread.c"
    "upstream/assimp/contrib/zlib/gzwrite.c"
    "upstream/assimp/contrib/zlib/inflate.c"
    "upstream/assimp/contrib/zlib/infback.c"
    "upstream/assimp/contrib/zlib/inftrees.c"
    "upstream/assimp/contrib/zlib/inffast.c"
    "upstream/assimp/contrib/zlib/trees.c"
    "upstream/assimp/contrib/zlib/uncompr.c"
    "upstream/assimp/contrib/zlib/zutil.c")

set(lucaria_external_assimp_sources
    ${assimp_zlib_sources}
    ${assimp_sources})

add_library(lucaria_external_assimp STATIC EXCLUDE_FROM_ALL ${lucaria_external_assimp_sources})
set_target_properties(lucaria_external_assimp PROPERTIES CXX_STANDARD 17)
set_target_properties(lucaria_external_assimp PROPERTIES C_STANDARD 99)
target_compile_definitions(lucaria_external_assimp PRIVATE -DRAPIDJSON_HAS_STDSTRING=1)

target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_X_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_AMF_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_3DS_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MD3_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MDL_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MD2_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_PLY_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_ASE_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_OBJ_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_HMP_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_SMD_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MDC_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MD5_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_STL_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_LWO_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_DXF_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_NFF_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_RAW_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_SIB_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_OFF_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_AC_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_BVH_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_IRRMESH_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_IRR_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_Q3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_B3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_COLLADA_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_TERRAGEN_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_CSM_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_LWS_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_OGRE_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_OPENGEX_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MS3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_COB_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_BLEND_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_Q3BSP_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_NDO_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_IFC_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_XGL_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_ASSBIN_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_C4D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_3MF_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_X3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_MMD_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_M3D_IMPORTER)
target_compile_definitions(lucaria_external_assimp PRIVATE ASSIMP_BUILD_NO_IQM_IMPORTER)

target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/openddlparser/code")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/openddlparser/include")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/pugixml/src")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/rapidjson/include")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/unzip")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/utf8cpp/source")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/contrib/zlib")
target_include_directories(lucaria_external_assimp PRIVATE "upstream/assimp/code")

target_include_directories(lucaria_external_assimp PUBLIC "include")
target_include_directories(lucaria_external_assimp PUBLIC "upstream/assimp/include")

if(MSVC)
    target_compile_options(lucaria_external_assimp PRIVATE /wd4244)
    target_compile_options(lucaria_external_assimp PRIVATE /wd4996)
endif()