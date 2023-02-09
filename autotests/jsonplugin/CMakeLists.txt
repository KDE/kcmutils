add_library(jsonplugin MODULE jsonplugin.cpp)

target_link_libraries(jsonplugin KF6::CoreAddons)

set_target_properties(jsonplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/testplugins")
