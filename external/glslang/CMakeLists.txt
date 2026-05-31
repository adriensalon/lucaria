set(GLSLANG_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/glslang")
file(MAKE_DIRECTORY "${GLSLANG_GENERATED_DIR}/glslang")

# glslang requires <glslang/build_info.h>
file(WRITE "${GLSLANG_GENERATED_DIR}/glslang/build_info.h" [[
#pragma once

#define GLSLANG_VERSION_MAJOR 16
#define GLSLANG_VERSION_MINOR 3
#define GLSLANG_VERSION_PATCH 0
#define GLSLANG_VERSION_FLAVOR ""

#define GLSLANG_VERSION_GREATER_THAN(major, minor, patch) \
((GLSLANG_VERSION_MAJOR) > (major) || ((major) == GLSLANG_VERSION_MAJOR && \
((GLSLANG_VERSION_MINOR) > (minor) || ((minor) == GLSLANG_VERSION_MINOR && \
	(GLSLANG_VERSION_PATCH) > (patch)))))

#define GLSLANG_VERSION_GREATER_OR_EQUAL_TO(major, minor, patch) \
((GLSLANG_VERSION_MAJOR) > (major) || ((major) == GLSLANG_VERSION_MAJOR && \
((GLSLANG_VERSION_MINOR) > (minor) || ((minor) == GLSLANG_VERSION_MINOR && \
	(GLSLANG_VERSION_PATCH >= (patch))))))

#define GLSLANG_VERSION_LESS_THAN(major, minor, patch) \
((GLSLANG_VERSION_MAJOR) < (major) || ((major) == GLSLANG_VERSION_MAJOR && \
((GLSLANG_VERSION_MINOR) < (minor) || ((minor) == GLSLANG_VERSION_MINOR && \
	(GLSLANG_VERSION_PATCH) < (patch)))))

#define GLSLANG_VERSION_LESS_OR_EQUAL_TO(major, minor, patch) \
((GLSLANG_VERSION_MAJOR) < (major) || ((major) == GLSLANG_VERSION_MAJOR && \
((GLSLANG_VERSION_MINOR) < (minor) || ((minor) == GLSLANG_VERSION_MINOR && \
	(GLSLANG_VERSION_PATCH <= (patch))))))
]])

set(GLSLANG_SOURCES
	"upstream/glslang/glslang/CInterface/glslang_c_interface.cpp"

	"upstream/glslang/glslang/GenericCodeGen/CodeGen.cpp"
	"upstream/glslang/glslang/GenericCodeGen/Link.cpp"

	"upstream/glslang/glslang/MachineIndependent/glslang_tab.cpp"
	"upstream/glslang/glslang/MachineIndependent/attribute.cpp"
	"upstream/glslang/glslang/MachineIndependent/Constant.cpp"
	"upstream/glslang/glslang/MachineIndependent/iomapper.cpp"
	"upstream/glslang/glslang/MachineIndependent/InfoSink.cpp"
	"upstream/glslang/glslang/MachineIndependent/Initialize.cpp"
	"upstream/glslang/glslang/MachineIndependent/IntermTraverse.cpp"
	"upstream/glslang/glslang/MachineIndependent/Intermediate.cpp"
	"upstream/glslang/glslang/MachineIndependent/ParseContextBase.cpp"
	"upstream/glslang/glslang/MachineIndependent/ParseHelper.cpp"
	"upstream/glslang/glslang/MachineIndependent/PoolAlloc.cpp"
	"upstream/glslang/glslang/MachineIndependent/RemoveTree.cpp"
	"upstream/glslang/glslang/MachineIndependent/Scan.cpp"
	"upstream/glslang/glslang/MachineIndependent/ShaderLang.cpp"
	"upstream/glslang/glslang/MachineIndependent/SpirvIntrinsics.cpp"
	"upstream/glslang/glslang/MachineIndependent/SymbolTable.cpp"
	"upstream/glslang/glslang/MachineIndependent/Versions.cpp"
	"upstream/glslang/glslang/MachineIndependent/intermOut.cpp"
	"upstream/glslang/glslang/MachineIndependent/limits.cpp"
	"upstream/glslang/glslang/MachineIndependent/linkValidate.cpp"
	"upstream/glslang/glslang/MachineIndependent/parseConst.cpp"
	"upstream/glslang/glslang/MachineIndependent/reflection.cpp"
	"upstream/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp"
	"upstream/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp"
	"upstream/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp"
	"upstream/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp"
	"upstream/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp"
	"upstream/glslang/glslang/MachineIndependent/propagateNoContraction.cpp"

	"upstream/glslang/SPIRV/GlslangToSpv.cpp"
	"upstream/glslang/SPIRV/InReadableOrder.cpp"
	"upstream/glslang/SPIRV/Logger.cpp"
	"upstream/glslang/SPIRV/SpvBuilder.cpp"
	"upstream/glslang/SPIRV/SpvPostProcess.cpp"
	"upstream/glslang/SPIRV/doc.cpp"
	"upstream/glslang/SPIRV/SpvTools.cpp"
	"upstream/glslang/SPIRV/disassemble.cpp"
	"upstream/glslang/SPIRV/CInterface/spirv_c_interface.cpp"

	"upstream/glslang/glslang/ResourceLimits/ResourceLimits.cpp"
	"upstream/glslang/glslang/ResourceLimits/resource_limits_c.cpp")

if(WIN32)
	list(APPEND GLSLANG_SOURCES	"upstream/glslang/glslang/OSDependent/Windows/ossource.cpp")
else()
	list(APPEND GLSLANG_SOURCES	"upstream/glslang/glslang/OSDependent/Unix/ossource.cpp")
endif()

add_library(lucaria_external_glslang STATIC ${GLSLANG_SOURCES})
target_include_directories(lucaria_external_glslang	PUBLIC "upstream/glslang" "${GLSLANG_GENERATED_DIR}")
target_compile_features(lucaria_external_glslang PUBLIC cxx_std_17)
target_compile_definitions(lucaria_external_glslang	PUBLIC ENABLE_SPIRV)
set_target_properties(lucaria_external_glslang PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
	target_compile_definitions(lucaria_external_glslang PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(lucaria_external_glslang PRIVATE
			/bigobj
			/wd4251
			/wd4244
			/wd4267
			/wd4275)
endif()

if(UNIX)
	find_package(Threads REQUIRED)
	target_link_libraries(lucaria_external_glslang PUBLIC Threads::Threads)
endif()