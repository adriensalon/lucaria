

add_library(lucaria_external_alsoft STATIC EXCLUDE_FROM_ALL "source/cereal.cpp")
set_target_properties(lucaria_external_alsoft PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_alsoft PUBLIC upstream/cereal/include)
