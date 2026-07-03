set(lucaria_external_meshoptimizer_sources
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/meshoptimizer.h"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/allocator.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/clusterizer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/indexanalyzer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/indexcodec.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/indexgenerator.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/meshletcodec.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/meshletutils.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/opacitymap.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/overdrawoptimizer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/partition.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/quantization.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/rasterizer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/simplifier.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/spatialorder.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/stripifier.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/tangentspace.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/vcacheoptimizer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/vertexcodec.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/vertexfilter.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src/vfetchoptimizer.cpp")
add_library(lucaria_external_meshoptimizer STATIC EXCLUDE_FROM_ALL ${lucaria_external_meshoptimizer_sources})
set_target_properties(lucaria_external_meshoptimizer PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_meshoptimizer PUBLIC "${CMAKE_CURRENT_LIST_DIR}/upstream/meshoptimizer/src")
