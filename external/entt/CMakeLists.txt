add_library(lucaria_external_entt STATIC EXCLUDE_FROM_ALL "source/entt.cpp")
set_target_properties(lucaria_external_entt PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_entt PUBLIC upstream/entt/src)
