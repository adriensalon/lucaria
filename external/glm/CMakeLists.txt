add_library(lucaria_external_glm STATIC EXCLUDE_FROM_ALL "source/glm.cpp")
set_target_properties(lucaria_external_glm PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_glm PUBLIC upstream/glm)
